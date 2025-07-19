// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPickerUtils.h"

FCollisionQueryParams FNPickerUtils::DefaultTraceParams = FCollisionQueryParams(FName(TEXT("NEXUS")),false);

FVector FNPickerUtils::DefaultProjection = FVector(0, 0, -500.f);
FRotator FNPickerUtils::DefaultRotation = FRotator::ZeroRotator;

FRotator FNPickerUtils::BaseRotation = FRotator(1,-1,0);
FMatrix FNPickerUtils::BaseMatrix = FRotationMatrix(BaseRotation);