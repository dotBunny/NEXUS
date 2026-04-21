// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NVoxelUtils.generated.h"

/**
 * Integer voxel coordinate triplet.
 *
 * Identifies a cell within a 3D voxel grid. Values are unsigned, so the origin voxel sits at (0,0,0).
 */
USTRUCT(BlueprintType)
struct FNVoxelCoordinate
{
	GENERATED_BODY()

	/** Grid index along the X axis. */
	UPROPERTY(VisibleAnywhere)
	uint32 X = 0;

	/** Grid index along the Y axis. */
	UPROPERTY(VisibleAnywhere)
	uint32 Y = 0;

	/** Grid index along the Z axis. */
	UPROPERTY(VisibleAnywhere)
	uint32 Z = 0;
};