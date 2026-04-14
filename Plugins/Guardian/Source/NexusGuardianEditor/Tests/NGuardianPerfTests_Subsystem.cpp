// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NGuardianSettings.h"
#include "NGuardianSubsystem.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::GuardianTests::Perf
{
    constexpr int32 IterationCount = 1000;
    constexpr float SetBaselineMaxDuration = 1.5f;   // SetBaseline reads GUObjectArray — allow generous budget
    constexpr float TickBelowMaxDuration   = 0.1f;   // Tick with no thresholds crossed is a cheap read
}

N_TEST_PERF(FNGuardianPerfTests_Subsystem_SetBaseline,
    "NEXUS::PerfTests::NGuardian::Subsystem::SetBaseline",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the cost of calling SetBaseline() IterationCount times.
    // SetBaseline reads GUObjectArray once and performs simple arithmetic —
    // this test establishes a baseline budget for monitoring overhead.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem) return;

        // Use a large threshold so no threshold events are triggered during the
        // timed region (which would cause log output and file I/O).
        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        const int32 OriginalWarning  = Settings->ObjectCountWarningThreshold;
        const int32 OriginalSnapshot = Settings->ObjectCountSnapshotThreshold;
        const int32 OriginalCompare  = Settings->ObjectCountCompareThreshold;
        Settings->ObjectCountWarningThreshold  = 99999999;
        Settings->ObjectCountSnapshotThreshold = 99999999;
        Settings->ObjectCountCompareThreshold  = 99999999;

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNGuardianPerfTests_Subsystem_SetBaseline,
                NEXUS::GuardianTests::Perf::SetBaselineMaxDuration)
            for (int32 i = 0; i < NEXUS::GuardianTests::Perf::IterationCount; ++i)
            {
                Subsystem->SetBaseline();
            }
            NTestTimer.ManualStop();
        }

        Settings->ObjectCountWarningThreshold  = OriginalWarning;
        Settings->ObjectCountSnapshotThreshold = OriginalSnapshot;
        Settings->ObjectCountCompareThreshold  = OriginalCompare;
        Subsystem->SetBaseline();
    }, true);
    FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNGuardianPerfTests_Subsystem_Tick_BelowThresholds,
    "NEXUS::PerfTests::NGuardian::Subsystem::Tick::BelowThresholds",
    N_TEST_CONTEXT_EDITOR)
{
    // Measures the per-frame monitoring cost when the object count is well below
    // all thresholds — the common-case path executed every game frame.
    FNTestUtils::PrePerformanceTest();
    FNTestUtils::WorldTest(EWorldType::PIE, [this](const UWorld* World)
    {
        UNGuardianSubsystem* Subsystem = UNGuardianSubsystem::Get(World);
        if (!Subsystem) return;

        UNGuardianSettings* Settings = UNGuardianSettings::GetMutable();
        const int32 OriginalWarning  = Settings->ObjectCountWarningThreshold;
        const int32 OriginalSnapshot = Settings->ObjectCountSnapshotThreshold;
        const int32 OriginalCompare  = Settings->ObjectCountCompareThreshold;
        Settings->ObjectCountWarningThreshold  = 99999999;
        Settings->ObjectCountSnapshotThreshold = 99999999;
        Settings->ObjectCountCompareThreshold  = 99999999;
        Subsystem->SetBaseline();

        // TEST
        {
            N_TEST_TIMER_SCOPE(FNGuardianPerfTests_Subsystem_Tick_BelowThresholds,
                NEXUS::GuardianTests::Perf::TickBelowMaxDuration)
            for (int32 i = 0; i < NEXUS::GuardianTests::Perf::IterationCount; ++i)
            {
                Subsystem->Tick(0.016f);
            }
            NTestTimer.ManualStop();
        }

        Settings->ObjectCountWarningThreshold  = OriginalWarning;
        Settings->ObjectCountSnapshotThreshold = OriginalSnapshot;
        Settings->ObjectCountCompareThreshold  = OriginalCompare;
        Subsystem->SetBaseline();
    }, true);
    FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
