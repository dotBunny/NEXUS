// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "NCellVoxelGenerationSettings.generated.h"

/**
 * Settings controlling how a cell's voxel occupancy data is generated from its level content.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellVoxelGenerationSettings
{
	GENERATED_BODY()

	/** When true, the generator consumes the voxel data; otherwise it is skipped entirely. */
	UPROPERTY(EditAnywhere)
	bool bUseVoxelData = false;

	/** When true, voxel data is recomputed automatically whenever the cell is saved. */
	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = false;

	/** When true, actors without collision still contribute to voxel occupancy. */
	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	/** When true, editor-only actors contribute to voxel occupancy. */
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	/**
	 * When true, landscapes contribute to voxel occupancy.
	 * @note Split from the Mesh Terrain flag below: the two are different kinds of actor with different reasons to be
	 *       refused. A landscape is an ordinary saved actor whose surface has to be sampled rather than read, where a
	 *       Mesh Terrain is transient and rebuilt.
	 */
	UPROPERTY(EditAnywhere)
	bool bIncludeLandscapes = true;

	/**
	 * When true, Mesh Terrain sections contribute to voxel occupancy.
	 * @note Governs both halves of the calculation at once, as its landscape counterpart does: whether the terrain
	 *       grows the voxel grid's extents, and whether the occupancy sweep can hit it. Excluded terrain joins the
	 *       ignored-actor list the sweep is issued with, so it cannot register as occupied even though the physics
	 *       world would otherwise report it.
	 */
	UPROPERTY(EditAnywhere)
	bool bIncludeMeshTerrains = true;

	/** Actors carrying any of these tags are excluded from the voxel calculation. */
	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { NEXUS::WorldAssembly::ActorTags::CellIgnore, NEXUS::WorldAssembly::ActorTags::CellVoxelIgnore };

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
		&& bIncludeLandscapes == Other.bIncludeLandscapes
		&& bIncludeMeshTerrains == Other.bIncludeMeshTerrains
		&& CollisionChannel == Other.CollisionChannel
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};