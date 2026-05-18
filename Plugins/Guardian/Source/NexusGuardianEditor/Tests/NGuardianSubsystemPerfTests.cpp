// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NGuardianSettings.h"
#include "NGuardianSubsystem.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NGuardian::UNGuardianSubsystemHarness
{
    constexpr int32 IterationCount = 1000;
    constexpr float SetBaselineMaxDuration = 5.0f;   // SetBaseline reads GUObjectArray and writes a formatted log line per call
    constexpr float TickBelowMaxDuration   = 0.1f;   // Tick with no thresholds crossed is a cheap read

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

class FNGuardianSubsystemPerfTests
{
public:
    // Measures the cost of calling SetBaseline() IterationCount times.
    // SetBaseline reads GUObjectArray once and performs simple arithmetic —
    // this test establishes a baseline budget for monitoring overhead.
    static void SetBaseline(UWorld* World)
    {
        using namespace NEXUS::PerfTests::NGuardian::UNGuardianSubsystemHarness;

        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem) return;

        FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        // Disable auto-baseline so the timer queued by OnWorldBeginPlay can't
        // fire during the timed region.
        Settings->bAutoBaseline = false;
        // Strictly increasing (Warning < Snapshot < Compare) is enforced by
        // SetBaseline; equal values cause an early-return and pollute the log.
        // Keep them large so no thresholds are crossed during measurement.
        Settings->ObjectCountWarningThreshold  = 99999997;
        Settings->ObjectCountSnapshotThreshold = 99999998;
        Settings->ObjectCountCompareThreshold  = 99999999;

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNGuardianPerfTests_Subsystem_SetBaseline,
                SetBaselineMaxDuration)
            for (int32 i = 0; i < IterationCount; ++i)
            {
                Subsystem->SetBaseline();
            }
            NTestTimer.ManualStop();
        }
    }

    // Measures the per-frame monitoring cost when the object count is well below
    // all thresholds — the common-case path executed every game frame.
    static void Tick_BelowThresholds(UWorld* World)
    {
        using namespace NEXUS::PerfTests::NGuardian::UNGuardianSubsystemHarness;

        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem) return;

        FSettingsGuard Guard;
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        Settings->bAutoBaseline = false;
        Settings->ObjectCountWarningThreshold  = 99999997;
        Settings->ObjectCountSnapshotThreshold = 99999998;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNGuardianPerfTests_Subsystem_Tick_BelowThresholds,
                TickBelowMaxDuration)
            for (int32 i = 0; i < IterationCount; ++i)
            {
                Subsystem->Tick(0.016f);
            }
            NTestTimer.ManualStop();
        }
    }
};

N_TEST_PERF(FNGuardianPerfTests_Subsystem_SetBaseline,
    "NEXUS::PerfTests::NGuardian::UNGuardianSubsystem::SetBaseline",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNGuardianSubsystemPerfTests::SetBaseline, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

N_TEST_PERF(FNGuardianPerfTests_Subsystem_Tick_BelowThresholds,
    "NEXUS::PerfTests::NGuardian::UNGuardianSubsystem::Tick::BelowThresholds",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNGuardianSubsystemPerfTests::Tick_BelowThresholds, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

#endif //WITH_TESTS
