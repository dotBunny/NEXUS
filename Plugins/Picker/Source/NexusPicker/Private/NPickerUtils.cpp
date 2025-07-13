// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPickerUtils.h"

FCollisionQueryParams FNPickerUtils::DefaultTraceParams = FCollisionQueryParams(FName(TEXT("NEXUS")),false);
FVector FNPickerUtils::DefaultCastBuffer = FVector(0, 0, -500.f);