// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyOperationSettings.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Types/NRawMesh.h"

/**
 * Snapshot of the target world's collision geometry used by graph-builder collision tests.
 *
 * Populated on the game thread by FNCreateVirtualWorldTask (world-collision data) and incrementally
 * by FNProcessPassTask (placed-cell hulls). All transforms are pre-resolved so that builder tasks
 * running on worker threads can run intersection queries without touching live UObjects.
 */
class FNVirtualWorldContext
{
public:
	/** Target world the snapshot was taken from. */
	TObjectPtr<UWorld> InputWorld;

	/** Spatial bounds of every organ participating in the operation, used to scope collision gathering. */
	TArray<FBoxSphereBounds> InputBounds;

	/** Per-element simple-collision meshes gathered from the target world during preprocess, stored in element-local space. */
	TArray<FNRawMesh> WorldCollisionMeshes;

	/** Initial captured transforms before baking */
	TArray<FTransform> WorldCollisionTransforms;

	/** Cell nodes already placed by earlier passes; each entry has matching mesh/location/rotation. */
	TArray<FNAssemblyGraphCellNode*> NodeIndex;

	/** Per-cell-node collision hulls, parallel array to NodeIndex. */
	TArray<FNRawMesh> NodeCollisionMeshes;

	/** World collision capture settings carried from the operation, controlling which actors are treated as collision sources. */
	FNWorldAssemblyWorldCollisionSettings WorldCollisionSettings;


	/** Context tags associated with this world context, added to during FNProcessPassTask collecting additions. */
	FGameplayTagContainer ContextTags;
	FNGameplayTagCounter TagCounter;

	explicit FNVirtualWorldContext(UWorld* TargetWorld, const TArray<FBoxSphereBounds>& TargetBounds, const FNAssemblyOperationSettings& Settings)
	{
		InputWorld = TargetWorld;
		InputBounds = TargetBounds;
		ContextTags = Settings.ContextTags;
		TagCounter = Settings.TagCounters;
		WorldCollisionSettings = Settings.WorldCollisionSettings;
	}
};
