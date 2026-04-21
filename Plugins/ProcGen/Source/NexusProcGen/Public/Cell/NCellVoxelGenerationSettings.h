// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NCellVoxelGenerationSettings.generated.h"

/**
 * Settings controlling how a cell's voxel occupancy data is generated from its level content.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellVoxelGenerationSettings
{
	GENERATED_BODY()

	/** When true, the generator consumes the voxel data; otherwise it is skipped entirely. */
	UPROPERTY(EditAnywhere)
	bool bUseVoxelData = false;

	/** When true, voxel data is recomputed automatically whenever the cell is saved. */
	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	/** When true, actors without collision still contribute to voxel occupancy. */
	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	/** When true, editor-only actors contribute to voxel occupancy. */
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	/** Actors carrying any of these tags are excluded from the voxel calculation. */
	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { "NCELL_VoxelIgnore" };

	/** Collision channel used when tracing against the cell to determine occupancy. */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldStatic;

	/** @return true if all fields match structurally; used when diffing live vs. side-car cell data. */
	bool Equals(const FNCellVoxelGenerationSettings& Other) const
	{
		return bUseVoxelData == Other.bUseVoxelData 
		&& bCalculateOnSave == Other.bCalculateOnSave
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly
		&& CollisionChannel == Other.CollisionChannel
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};