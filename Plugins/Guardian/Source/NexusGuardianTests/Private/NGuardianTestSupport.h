// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "CoreMinimal.h"
#include "NGuardianSettings.h"

namespace NEXUS::Testing::NGuardian::UNGuardianSubsystemHarness
{
    // RAII guard that saves and restores every UNGuardianSettings field so individual
    // tests can modify them without polluting subsequent tests. Shared by the Guardian
    // subsystem unit and performance tests; keep the field set in sync with UNGuardianSettings.
    struct FSettingsGuard
    {
        int32 OriginalWarning;
        int32 OriginalSnapshot;
        int32 OriginalCompare;
        bool  OriginalCaptureOutput;
        bool  OriginalAutoBaseline;
        float OriginalAutoBaselineDelay;
        float OriginalTickRate;
        uint8 OriginalBuildAvailability;

        FSettingsGuard()
        {
            const UNGuardianSettings* S = UNGuardianSettings::Get();
            OriginalWarning           = S->ObjectCountWarningThreshold;
            OriginalSnapshot          = S->ObjectCountSnapshotThreshold;
            OriginalCompare           = S->ObjectCountCompareThreshold;
            OriginalCaptureOutput     = S->bObjectCountCaptureOutput;
            OriginalAutoBaseline      = S->bAutoBaseline;
            OriginalAutoBaselineDelay = S->AutoBaselineDelay;
            OriginalTickRate          = S->TickRate;
            OriginalBuildAvailability = S->BuildAvailability;
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
            S->TickRate                     = OriginalTickRate;
            S->BuildAvailability            = OriginalBuildAvailability;
        }
    };
}

#endif // WITH_TESTS
