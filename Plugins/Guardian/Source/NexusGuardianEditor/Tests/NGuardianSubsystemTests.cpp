// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "NGuardianSettings.h"
#include "NGuardianSubsystem.h"
#include "TimerManager.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness
{
    // RAII guard that saves and restores UNGuardianSettings fields so individual
    // tests can modify them without polluting subsequent tests.
    struct FSettingsGuard
    {
        int32 OriginalWarning;
        int32 OriginalSnapshot;
        int32 OriginalCompare;
        bool  OriginalCaptureOutput;
        bool  OriginalAutoBaseline;
        float OriginalAutoBaselineDelay;

        FSettingsGuard()
        {
            const UNGuardianSettings* S = UNGuardianSettings::Get();
            OriginalWarning           = S->ObjectCountWarningThreshold;
            OriginalSnapshot          = S->ObjectCountSnapshotThreshold;
            OriginalCompare           = S->ObjectCountCompareThreshold;
            OriginalCaptureOutput     = S->bObjectCountCaptureOutput;
            OriginalAutoBaseline      = S->bAutoBaseline;
            OriginalAutoBaselineDelay = S->AutoBaselineDelay;
        }

        ~FSettingsGuard()
        {
            UNGuardianSettings* S = UNGuardianSettings::GetMutable();
            S->ObjectCountWarningThreshold  = OriginalWarning;
            S->ObjectCountSnapshotThreshold = OriginalSnapshot;
            S->ObjectCountCompareThreshold  = OriginalCompare;
            S->bObjectCountCaptureOutput    = OriginalCaptureOutput;
            S->bAutoBaseline                = OriginalAutoBaseline;
            S->AutoBaselineDelay            = OriginalAutoBaselineDelay;
        }
    };
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_SetBaseline_InitialState,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::SetBaseline::InitialState",
    N_TEST_CONTEXT_EDITOR)
{
    // OnWorldBeginPlay schedules SetBaseline() on a timer (UNGuardianSettings::AutoBaselineDelay),
    // so this test calls SetBaseline() explicitly to verify its post-conditions: all threshold
    // flags false, a positive base count, and IsTickable() true.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

        Subsystem->SetBaseline();

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

N_TEST_HIGH(UNGuardianSubsystemTests_AutoBaseline_Disabled,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::AutoBaseline::Disabled",
    N_TEST_CONTEXT_EDITOR)
{
    // With bAutoBaseline=false, OnWorldBeginPlay must skip scheduling the timer, so the
    // subsystem stays non-tickable until something calls SetBaseline() manually.
    using namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness;

    FSettingsGuard Guard;
    UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
    Settings->bAutoBaseline = false;

    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

        World->GetTimerManager().Tick(1.0f);

        if (Subsystem->IsTickable())
        {
            ADD_ERROR("IsTickable should be false when bAutoBaseline=false and SetBaseline has not been called.");
        }
        CHECK_EQUALS("BaseObjectCount should remain zero when auto-baseline is disabled.",
            Subsystem->GetBaseObjectCount(), 0)
    });
}

N_TEST_HIGH(UNGuardianSubsystemTests_AutoBaseline_FiresAfterDelay,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::AutoBaseline::FiresAfterDelay",
    N_TEST_CONTEXT_EDITOR)
{
    // With bAutoBaseline=true and a tiny AutoBaselineDelay, OnWorldBeginPlay schedules a
    // timer that fires on the next timer-manager tick — the subsystem must become tickable
    // with a positive base count once the timer has run.
    using namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness;

    FSettingsGuard Guard;
    UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
    Settings->bAutoBaseline = true;
    // FTimerManager silently discards SetTimer with Rate==0, so use a tiny positive delay
    // and tick the manager past it to fire the timer.
    Settings->AutoBaselineDelay = 0.001f;

    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

        // When SetTimer runs before the manager has been ticked this frame, the new timer
        // is queued in PendingTimerSet — it doesn't reach ActiveTimerHeap until the END of
        // the next Tick(). So the first tick promotes it; only the second tick fires it.
        // Bump GFrameCounter between ticks so HasBeenTickedThisFrame() lets each one run.
        ++GFrameCounter;
        World->GetTimerManager().Tick(0.1f);
        ++GFrameCounter;
        World->GetTimerManager().Tick(0.1f);

        if (!Subsystem->IsTickable())
        {
            ADD_ERROR("IsTickable should be true after the auto-baseline timer fires.");
        }
        if (Subsystem->GetBaseObjectCount() <= 0)
        {
            ADD_ERROR("BaseObjectCount should be positive after the auto-baseline timer fires.");
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
        Subsystem->UpdateTickRate(0.f);

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness::FSettingsGuard Guard;
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
        Subsystem->UpdateTickRate(0.f);

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
        Subsystem->UpdateTickRate(0.f);

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();

        // Negative offsets so the current object count already meets the warning threshold;
        // SetBaseline enforces strict Warning < Snapshot < Compare ordering.
        Settings->ObjectCountWarningThreshold  = -3;
        Settings->ObjectCountSnapshotThreshold = -2;
        Settings->ObjectCountCompareThreshold  = -1;
        Subsystem->SetBaseline();

        // Tick once to set the warning flag (returns early after setting it).
        this->AddExpectedError(TEXT("warning threshold has been met"), EAutomationExpectedErrorFlags::Contains, 1);
        Subsystem->Tick(0.f);
        if (!Subsystem->HasPassedWarningThreshold())
        {
            ADD_ERROR("Warning flag should be set after first Tick with a negative-offset threshold.");
            return;
        }

        // Now switch to thresholds well above the current count and re-baseline. All flags must be reset.
        Settings->ObjectCountWarningThreshold  = 99999997;
        Settings->ObjectCountSnapshotThreshold = 99999998;
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
    // With a negative warning offset, the first Tick() must set the warning flag.
    // The snapshot and compare flags must remain false (each needs a separate Tick).
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

    	// We expect some messages
		AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);

        NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        // SetBaseline enforces strict Warning < Snapshot < Compare ordering, so the two
        // un-triggered thresholds must still be strictly larger than warning.
        Settings->ObjectCountWarningThreshold  = -1;
        Settings->ObjectCountSnapshotThreshold = 99999998;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        Subsystem->Tick(0.f);

        CHECK_MESSAGE("Warning flag must be true after one Tick at a negative-offset threshold.", Subsystem->HasPassedWarningThreshold())
        CHECK_FALSE_MESSAGE("Snapshot flag must remain false — each threshold needs its own Tick.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_FALSE_MESSAGE("Compare flag must remain false — each threshold needs its own Tick.", Subsystem->HasPassedCompareThreshold())
    }, true);
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_Tick_SnapshotThreshold,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::SnapshotThreshold",
    N_TEST_CONTEXT_EDITOR)
{
    // With warning and snapshot offsets below the current count, two Ticks must set both
    // flags. The compare flag must remain false.
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        using namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness;

        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

    	// We expect some messages
		AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);
		AddExpectedMessage(TEXT("The UObject count snapshot threshold has been met"), ELogVerbosity::Error);
		AddExpectedMessage(TEXT("A UObject snapshot has been written to"), ELogVerbosity::Error);

        FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        // Strict Warning < Snapshot < Compare required by SetBaseline; negative offsets keep
        // both triggerable thresholds below the live UObject count.
        Settings->ObjectCountWarningThreshold  = -2;
        Settings->ObjectCountSnapshotThreshold = -1;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Settings->bObjectCountCaptureOutput    = true;
        Subsystem->SetBaseline();

        const TSet<FString> PreSnapshotFiles = FNTestUtils::SnapshotLogFiles(TEXT("NEXUS_Snapshot_*.txt"));

        Subsystem->Tick(0.f); // triggers warning, returns early
        Subsystem->Tick(0.f); // triggers snapshot, returns early

        CHECK_MESSAGE("Warning flag must be true after two Ticks.", Subsystem->HasPassedWarningThreshold())
        CHECK_MESSAGE("Snapshot flag must be true after two Ticks at zero-offset thresholds.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_FALSE_MESSAGE("Compare flag must remain false — needs a third Tick.", Subsystem->HasPassedCompareThreshold())

        const FString SnapshotPath = FNTestUtils::WaitForNewLogFile(TEXT("NEXUS_Snapshot_*.txt"), PreSnapshotFiles);
        if (SnapshotPath.IsEmpty())
        {
            ADD_ERROR("Snapshot file was not written to ProjectLogDir within the timeout.");
        }
        else
        {
            if (IFileManager::Get().FileSize(*SnapshotPath) <= 0)
            {
                ADD_ERROR(FString::Printf(TEXT("Snapshot file %s is empty."), *SnapshotPath));
            }
            IFileManager::Get().Delete(*SnapshotPath);
        }
    }, true);
}

N_TEST_CRITICAL(UNGuardianSubsystemTests_Tick_CompareThreshold,
    "NEXUS::UnitTests::NGuardian::UNGuardianSubsystem::Tick::CompareThreshold",
    N_TEST_CONTEXT_EDITOR)
{
    // With all three offsets below the current count and strictly increasing, three Ticks
    // must set all three flags. Crossing the snapshot and compare thresholds asynchronously
    // writes files to ProjectLogDir; the test polls for both files to verify the writes
    // actually completed.
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        using namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness;

        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

        FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = -3;
        Settings->ObjectCountSnapshotThreshold = -2;
        Settings->ObjectCountCompareThreshold  = -1;
        Settings->bObjectCountCaptureOutput    = true;
        Subsystem->SetBaseline();

        const TSet<FString> PreSnapshotFiles = FNTestUtils::SnapshotLogFiles(TEXT("NEXUS_Snapshot_*.txt"));
        const TSet<FString> PreCompareFiles  = FNTestUtils::SnapshotLogFiles(TEXT("NEXUS_Compare_*.txt"));

    	AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);
        Subsystem->Tick(0.f); // warning

    	AddExpectedMessage(TEXT("The UObject count snapshot threshold has been met"), ELogVerbosity::Error);
    	AddExpectedMessage(TEXT("A UObject snapshot has been written to"), ELogVerbosity::Error);
    	Subsystem->Tick(0.f); // snapshot

    	AddExpectedMessage(TEXT("The UObject count compare threshold has been met"), ELogVerbosity::Error);
    	AddExpectedMessage(TEXT("A UObject comparison written to"), ELogVerbosity::Error);
    	Subsystem->Tick(0.f); // compare

        CHECK_MESSAGE("Warning flag must be true after three Ticks.", Subsystem->HasPassedWarningThreshold())
        CHECK_MESSAGE("Snapshot flag must be true after three Ticks.", Subsystem->HasPassedSnapshotThreshold())
        CHECK_MESSAGE("Compare flag must be true after three Ticks at below-count thresholds.", Subsystem->HasPassedCompareThreshold())

        const FString SnapshotPath = FNTestUtils::WaitForNewLogFile(TEXT("NEXUS_Snapshot_*.txt"), PreSnapshotFiles);
        const FString ComparePath  = FNTestUtils::WaitForNewLogFile(TEXT("NEXUS_Compare_*.txt"),  PreCompareFiles);

        if (SnapshotPath.IsEmpty())
        {
            ADD_ERROR("Snapshot file was not written to ProjectLogDir within the timeout.");
        }
        else
        {
            if (IFileManager::Get().FileSize(*SnapshotPath) <= 0)
            {
                ADD_ERROR(FString::Printf(TEXT("Snapshot file %s is empty."), *SnapshotPath));
            }
            IFileManager::Get().Delete(*SnapshotPath);
        }

        if (ComparePath.IsEmpty())
        {
            ADD_ERROR("Compare file was not written to ProjectLogDir within the timeout.");
        }
        else
        {
            if (IFileManager::Get().FileSize(*ComparePath) <= 0)
            {
                ADD_ERROR(FString::Printf(TEXT("Compare file %s is empty."), *ComparePath));
            }
            IFileManager::Get().Delete(*ComparePath);
        }
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
        using namespace NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness;

        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNGuardianSubsystem from PIE world.");
            return;
        }
        Subsystem->UpdateTickRate(0.f);

        FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = -3;
        Settings->ObjectCountSnapshotThreshold = -2;
        Settings->ObjectCountCompareThreshold  = -1;
        Settings->bObjectCountCaptureOutput    = true;
        Subsystem->SetBaseline();

        const TSet<FString> PreSnapshotFiles = FNTestUtils::SnapshotLogFiles(TEXT("NEXUS_Snapshot_*.txt"));
        const TSet<FString> PreCompareFiles  = FNTestUtils::SnapshotLogFiles(TEXT("NEXUS_Compare_*.txt"));

    	// We expect some messages
    	AddExpectedMessage(TEXT("The UObject count warning threshold has been met"), ELogVerbosity::Warning);
		AddExpectedMessage(TEXT("The UObject count snapshot threshold has been met"), ELogVerbosity::Error);
		AddExpectedMessage(TEXT("A UObject snapshot has been written to"), ELogVerbosity::Error);
    	AddExpectedMessage(TEXT("The UObject count compare threshold has been met"), ELogVerbosity::Error);
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

        const FString SnapshotPath = FNTestUtils::WaitForNewLogFile(TEXT("NEXUS_Snapshot_*.txt"), PreSnapshotFiles);
        const FString ComparePath  = FNTestUtils::WaitForNewLogFile(TEXT("NEXUS_Compare_*.txt"),  PreCompareFiles);

        if (SnapshotPath.IsEmpty())
        {
            ADD_ERROR("Snapshot file was not written to ProjectLogDir within the timeout.");
        }
        else
        {
            if (IFileManager::Get().FileSize(*SnapshotPath) <= 0)
            {
                ADD_ERROR(FString::Printf(TEXT("Snapshot file %s is empty."), *SnapshotPath));
            }
            IFileManager::Get().Delete(*SnapshotPath);
        }

        if (ComparePath.IsEmpty())
        {
            ADD_ERROR("Compare file was not written to ProjectLogDir within the timeout.");
        }
        else
        {
            if (IFileManager::Get().FileSize(*ComparePath) <= 0)
            {
                ADD_ERROR(FString::Printf(TEXT("Compare file %s is empty."), *ComparePath));
            }
            IFileManager::Get().Delete(*ComparePath);
        }
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
        Subsystem->UpdateTickRate(0.f);

        // Use large, strictly-increasing thresholds so no flags are triggered.
        NEXUS::UnitTests::NGuardian::UNGuardianSubsystemHarness::FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->ObjectCountWarningThreshold  = 99999997;
        Settings->ObjectCountSnapshotThreshold = 99999998;
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
