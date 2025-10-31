// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellBoundsGenerationSettings.h"
#include "NCellHullGenerationSettings.h"
#include "NCellVoxelGenerationSettings.h"
#include "NColor.h"
#include "Types/NRawMesh.h"
#include "Types/NRotationConstraint.h"
#include "NCellVoxelData.h"

#include "NCellRootDetails.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellRootDetails
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FNCellBoundsGenerationSettings BoundsSettings;
	 
	UPROPERTY(VisibleAnywhere)
	FBox Bounds;

	UPROPERTY(VisibleAnywhere)
	FBox UnitBounds;

	UPROPERTY(EditAnywhere)
	FNRotationConstraint RotationConstraints;

	UPROPERTY(EditAnywhere)
	FNCellHullGenerationSettings HullSettings;
	
	UPROPERTY(VisibleAnywhere)
	FNRawMesh Hull;
	
	UPROPERTY(EditAnywhere)
	FLinearColor ProxyColor = FNColor::NexusDarkBlue;

	UPROPERTY(EditAnywhere)
	FNCellVoxelGenerationSettings VoxelSettings;
	
	UPROPERTY(EditAnywhere)
	FNCellVoxelData VoxelData;
	
	bool CopyTo(FNCellRootDetails& Other) const
	{
		Other = *this;
		return true;
	}

	bool IsEqual(const FNCellRootDetails& Other) const
	{
		return ProxyColor == Other.ProxyColor
		&& Bounds == Other.Bounds
		&& BoundsSettings.Equals(Other.BoundsSettings)
		&& HullSettings.Equals(Other.HullSettings)
		&& Hull.IsEqual(Other.Hull)
		&& VoxelSettings.Equals(Other.VoxelSettings)
		&& VoxelData.IsEqual(Other.VoxelData)
		&& RotationConstraints.IsEqual(Other.RotationConstraints);
	}
};

