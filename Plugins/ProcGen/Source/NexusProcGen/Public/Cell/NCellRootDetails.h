// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellBoundsGenerationSettings.h"
#include "NCellHullGenerationSettings.h"
#include "NCellVoxelGenerationSettings.h"
#include "NColor.h"
#include "Types/NRawMesh.h"
#include "Types/NRotationConstraints.h"
#include "NCellVoxelData.h"

#include "NCellRootDetails.generated.h"

/**
 * Persistent, on-disk cell root state.
 *
 * Stored both on UNCellRootComponent (live in the level) and on UNCell (side-car asset) so the two can
 * be diffed and synced without requiring the full level to be loaded.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellRootDetails
{
	GENERATED_BODY()

	/** Input settings used when recomputing Bounds. */
	UPROPERTY(EditAnywhere)
	FNCellBoundsGenerationSettings BoundsSettings;

	/** World-space axis-aligned bounds of the cell content, derived from BoundsSettings. */
	UPROPERTY(VisibleAnywhere)
	FBox Bounds;

	/** Bounds quantized to UnitSize — used when aligning cells onto a grid during generation. */
	UPROPERTY(VisibleAnywhere)
	FBox UnitBounds;

	/** Derived unit size (one grid cell) implied by Bounds and project voxel size. */
	UPROPERTY(VisibleAnywhere)
	FVector UnitSize = FVector::ZeroVector;

	/** Allowed rotations for this cell when placed by the generator. */
	UPROPERTY(EditAnywhere)
	FNRotationConstraints RotationConstraints;

	/** Input settings used when recomputing Hull. */
	UPROPERTY(EditAnywhere)
	FNCellHullGenerationSettings HullSettings;

	/** Convex/concave hull derived from the cell's geometry. */
	UPROPERTY(VisibleAnywhere)
	FNRawMesh Hull;

	/** Color used by the cell proxy while the level instance is still streaming in. */
	UPROPERTY(EditAnywhere)
	FLinearColor ProxyColor = FNColor::NexusDarkBlue;

	/** Input settings used when recomputing VoxelData. */
	UPROPERTY(EditAnywhere)
	FNCellVoxelGenerationSettings VoxelSettings;

	/** Voxelized occupancy data for the cell, used by graph building and collision reasoning. */
	UPROPERTY(EditAnywhere)
	FNCellVoxelData VoxelData;

	/** Copy this struct into Other by value. @return Always true. */
	bool CopyTo(FNCellRootDetails& Other) const
	{
		Other = *this;
		return true;
	}

	/** @return true if every field matches structurally; used to detect author-time drift from the side-car. */
	bool IsEqual(const FNCellRootDetails& Other) const
	{
		return  ProxyColor == Other.ProxyColor &&
				Bounds == Other.Bounds &&
				BoundsSettings.Equals(Other.BoundsSettings) &&
				UnitBounds == Other.UnitBounds &&
				UnitSize == Other.UnitSize &&
				HullSettings.Equals(Other.HullSettings) &&
				Hull.IsEqual(Other.Hull) &&
				VoxelSettings.Equals(Other.VoxelSettings) &&
				VoxelData.IsEqual(Other.VoxelData) &&
				RotationConstraints.IsEqual(Other.RotationConstraints);
	}
};

