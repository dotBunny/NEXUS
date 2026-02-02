// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPickerUtils.h"

FCollisionQueryParams FNPickerUtils::CollisionQueryParams = FCollisionQueryParams(FName(TEXT("NEXUS")),false);
FVector FNPickerUtils::NavQueryExtent = FVector(1500, 1500, 1500);
FNavAgentProperties FNPickerUtils::NavAgentProperties = FNavAgentProperties(1, 10 );