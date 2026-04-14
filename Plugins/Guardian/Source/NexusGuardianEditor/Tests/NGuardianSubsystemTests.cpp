// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NGuardianSettings.h"
#include "NGuardianSubsystem.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystem
{
    // RAII guard that saves and restores UNGuardianSettings threshold offsets so
    // individual tests can modify them without polluting subsequent tests.
    struct FSettingsGuard
    {
        int32 OriginalWarning;
        int32 OriginalSnapshot;
        int32 OriginalCompare;

        FSettingsGuard()
        {
            const UNGuardianSettings* S = UNGuardianSettings::Get();
            OriginalWarning  = S->ObjectCountWarningThreshold;
            OriginalSnapshot = S->ObjectCountSnapshotThreshold;
            OriginalCompare  = S->ObjectCountCompareThreshold;
        }

        ~FSettingsGuard()
        {
            UNGuardianSettings* S = UNGuardianSettings::GetMutable();
            S->ObjectCountWarningThreshold  = OriginalWarning;
            S->ObjectCountSnapshotThreshold = OriginalSnapshot;
            S->ObjectCountCompareThreshold  = OriginalCompare;
        }
    };
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_SetBaseline_InitialState,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::SetBaseline::InitialState",
    N_TEST_CONTEXT_EDITOR)
{
    // OnWorldBeginPlay calls SetBaseline() automatically. Verify the post-conditions:
    // all threshold flags false, a positive base count, and IsTickable() true.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

        CHECK_FALSE_MESSAGE("Warning threshold flag should be false after SetBaseline.", Subsystem->HasPassedWarningThreshold())
        CHECK_FALSE_MESSAGE("Snapshot threshold flag should be false after SetBaseline.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_FALSE_MESSAGE("Compare threshold flag should be false after SetBaseline.", Subsystem->HasPassedCompareThreshold())

        if (Subsystem->GetBaseObjectCount() <= 0)
        {
            ADD_ERROR("BaseObjectCount should be positive after SetBaseline.");
        }

        if (!Subsystem->IsTickable())
        {
            ADD_ERROR("IsTickable should return true once a baseline has been set.");
        }
    });
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_SetBaseline_ThresholdCalculation,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::SetBaseline::ThresholdCalculation",
    N_TEST_CONTEXT_EDITOR)
{
    // Thresholds must equal BaseObjectCount + the respective settings offset.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 100;
        Settings->ObjectCountSnapshotThreshold = 200;
        Settings->ObjectCountCompareThreshold  = 400;

        Subsystem->SetBaseline();

        const int32 Base = Subsystem->GetBaseObjectCount();
        CHECK_EQUALS("Warning threshold should be base + settings offset.",
            Subsystem->GetObjectCountWarningThreshold(),  Base + 100)
        CHECK_EQUALS("Snapshot threshold should be base + settings offset.",
            Subsystem->GetObjectCountSnapshotThreshold(), Base + 200)
        CHECK_EQUALS("Compare threshold should be base + settings offset.",
            Subsystem->GetObjectCountCompareThreshold(),  Base + 400)
    });
}

N_TEST_HIGH(UNGuardianSubsystemTests_SetBaseline_ThresholdOrdering,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::SetBaseline::ThresholdOrdering",
    N_TEST_CONTEXT_EDITOR)
{
    // With default settings (Warning < Snapshot < Compare offsets) the absolute
    // thresholds must maintain the same strict ordering.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

        Subsystem->SetBaseline();

        const int32 Warning  = Subsystem->GetObjectCountWarningThreshold();
        const int32 Snapshot = Subsystem->GetObjectCountSnapshotThreshold();
        const int32 Compare  = Subsystem->GetObjectCountCompareThreshold();

        if (Warning >= Snapshot)
        {
            ADD_ERROR("Warning threshold must be less than Snapshot threshold.");
        }
        if (Snapshot >= Compare)
        {
            ADD_ERROR("Snapshot threshold must be less than Compare threshold.");
        }
    });
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_SetBaseline_ResetsFlags,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::SetBaseline::ResetsFlags",
    N_TEST_CONTEXT_EDITOR)
{
    // After triggering the warning threshold via Tick(), calling SetBaseline()
    // again must reset all flags to false.
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();

        // Set a zero offset so the current object count already meets the threshold.
        Settings->ObjectCountWarningThreshold  = 0;
        Settings->ObjectCountSnapshotThreshold = 0;
        Settings->ObjectCountCompareThreshold  = 0;
        Subsystem->SetBaseline();

        // Tick once to set the warning flag (returns early after setting it).
        this->AddExpectedError(TEXT("warning threshold has been met"), EAutomationExpectedErrorFlags::Contains, 1);
        Subsystem->Tick(0.f);
        if (!Subsystem->HasPassedWarningThreshold())
        {
            ADD_ERROR("Warning flag should be set after first Tick with a zero-offset threshold.");
            return;
        }

        // Now restore large thresholds and re-baseline. All flags must be reset.
        Settings->ObjectCountWarningThreshold  = 99999999;
        Settings->ObjectCountSnapshotThreshold = 99999999;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        CHECK_FALSE_MESSAGE("Warning flag must be false after SetBaseline.", Subsystem->HasPassedWarningThreshold())
        CHECK_FALSE_MESSAGE("Snapshot flag must be false after SetBaseline.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_FALSE_MESSAGE("Compare flag must be false after SetBaseline.", Subsystem->HasPassedCompareThreshold())
    }, true);
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_Tick_WarningThreshold,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::WarningThreshold",
    N_TEST_CONTEXT_EDITOR)
{
    // With a zero warning offset, the first Tick() must set the warning flag.
    // The snapshot and compare flags must remain false (each needs a separate Tick).
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
    	
    	// We expect some messages
		AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 0;
        Settings->ObjectCountSnapshotThreshold = 99999999;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        Subsystem->Tick(0.f);

        CHECK_MESSAGE("Warning flag must be true after one Tick at a zero-offset threshold.", Subsystem->HasPassedWarningThreshold())
        CHECK_FALSE_MESSAGE("Snapshot flag must remain false — each threshold needs its own Tick.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_FALSE_MESSAGE("Compare flag must remain false — each threshold needs its own Tick.", Subsystem->HasPassedCompareThreshold())
    }, true);
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_Tick_SnapshotThreshold,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::SnapshotThreshold",
    N_TEST_CONTEXT_EDITOR)
{
    // With zero offsets for warning and snapshot, two Ticks must set both flags.
    // The compare flag must remain false.
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

    	// We expect some messages
		AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);
		AddExpectedMessage(TEXT("The UObject count snapshot threshold has been met"), ELogVerbosity::Error);
    	
        NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 0;
        Settings->ObjectCountSnapshotThreshold = 0;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        Subsystem->Tick(0.f); // triggers warning, returns early
        Subsystem->Tick(0.f); // triggers snapshot, returns early

        CHECK_MESSAGE("Warning flag must be true after two Ticks.", Subsystem->HasPassedWarningThreshold())
        CHECK_MESSAGE("Snapshot flag must be true after two Ticks at zero-offset thresholds.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_FALSE_MESSAGE("Compare flag must remain false — needs a third Tick.", Subsystem->HasPassedCompareThreshold())
    }, true);
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_Tick_CompareThreshold,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::CompareThreshold",
    N_TEST_CONTEXT_EDITOR)
{
    // With all three offsets at zero, three Ticks must set all three flags.
    // Note: crossing the compare threshold always writes a diff file to ProjectLogDir.
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

		NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 0;
        Settings->ObjectCountSnapshotThreshold = 0;
        Settings->ObjectCountCompareThreshold  = 0;
        Subsystem->SetBaseline();

    	AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);
        Subsystem->Tick(0.f); // warning
        
    	AddExpectedMessage(TEXT("The UObject count snapshot threshold has been met"), ELogVerbosity::Error);
    	Subsystem->Tick(0.f); // snapshot
    	
    	AddExpectedMessage(TEXT("The UObject count compare threshold met"), ELogVerbosity::Error);
    	AddExpectedMessage(TEXT("A UObject comparison written to"), ELogVerbosity::Error);
    	Subsystem->Tick(0.f); // compare

        CHECK_MESSAGE("Warning flag must be true after three Ticks.", Subsystem->HasPassedWarningThreshold())
        CHECK_MESSAGE("Snapshot flag must be true after three Ticks.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_MESSAGE("Compare flag must be true after three Ticks at zero-offset thresholds.", Subsystem->HasPassedCompareThreshold())
    }, true);
}

N_TEST_HIGH(UNGuardianSubsystemTests_Tick_FlagsStableOnceSet,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::FlagsStableOnceSet",
    N_TEST_CONTEXT_EDITOR)
{
    // Once all three flags are set, additional Ticks must not change them (as long
    // as the object count stays at or above the warning threshold).
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 0;
        Settings->ObjectCountSnapshotThreshold = 0;
        Settings->ObjectCountCompareThreshold  = 0;
        Subsystem->SetBaseline();

    	// We expect some messages
    	AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);
		AddExpectedMessage(TEXT("The UObject count snapshot threshold has been met"), ELogVerbosity::Error);
    	AddExpectedMessage(TEXT("The UObject count compare threshold met"), ELogVerbosity::Error);
		AddExpectedMessage(TEXT("A UObject comparison written to"), ELogVerbosity::Error);
    	
        // Advance through all three thresholds.
        Subsystem->Tick(0.f);
        Subsystem->Tick(0.f);
        Subsystem->Tick(0.f);

        // Additional ticks must be no-ops.
        Subsystem->Tick(0.f);
        Subsystem->Tick(0.f);

        CHECK_MESSAGE("Warning flag must remain true after extra Ticks.", Subsystem->HasPassedWarningThreshold())
        CHECK_MESSAGE("Snapshot flag must remain true after extra Ticks.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_MESSAGE("Compare flag must remain true after extra Ticks.", Subsystem->HasPassedCompareThreshold())
    }, true);
}

N_TEST_HIGH(UNGuardianSubsystemTests_Tick_LastObjectCountUpdated,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::LastObjectCountUpdated",
    N_TEST_CONTEXT_EDITOR)
{
    // GetLastObjectCount() must reflect the most recent object count read during Tick.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }

        // Use a large threshold so no flags are triggered.
        NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 99999999;
        Settings->ObjectCountSnapshotThreshold = 99999999;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        Subsystem->Tick(0.f);

        if (Subsystem->GetLastObjectCount() <= 0)
        {
            ADD_ERROR("GetLastObjectCount should be positive after Tick.");
        }
    });
}

#endif //WITH_TESTS
