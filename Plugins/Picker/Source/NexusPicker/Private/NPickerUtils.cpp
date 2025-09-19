﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPickerUtils.h"

DEFINE_LOG_CATEGORY(LogNexusPicker);

FCollisionQueryParams FNPickerUtils::DefaultTraceParams = FCollisionQueryParams(FName(TEXT("NEXUS")),false);

bool FNPickerUtils::bVisualLoggingEnabled = true;

FVector FNPickerUtils::DefaultProjection = FVector(0, 0, -500.f);
FRotator FNPickerUtils::DefaultRotation = FRotator::ZeroRotator;

FRotator FNPickerUtils::BaseRotation = FRotator(1,-1,0);
FMatrix FNPickerUtils::BaseMatrix = FRotationMatrix(BaseRotation);