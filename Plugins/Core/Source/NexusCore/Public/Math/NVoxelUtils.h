// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NVoxelUtils.generated.h"

USTRUCT(BlueprintType)
struct FNVoxelCoordinate
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	uint32 X = 0;
	
	UPROPERTY(VisibleAnywhere)
	uint32 Y = 0;
	
	UPROPERTY(VisibleAnywhere)
	uint32 Z = 0;
};