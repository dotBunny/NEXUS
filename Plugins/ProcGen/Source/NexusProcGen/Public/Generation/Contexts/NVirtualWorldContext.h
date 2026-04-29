// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Graph/NProcGenGraphCellNode.h"
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
	UWorld* InputWorld;

	/** Spatial bounds of every organ participating in the operation, used to scope collision gathering. */
	TArray<FBoxSphereBounds> InputBounds;

	/** Per-element simple-collision meshes gathered from the target world during preprocess, stored in element-local space. */
	TArray<FNRawMesh> WorldCollisionMeshes;

	/** World-space transforms corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FTransform> WorldCollisionMeshTransforms;

	/** World-space location corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FVector> WorldCollisionMeshLocations;

	/** World-space rotation corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FRotator> WorldCollisionMeshRotations;


	/** Cell nodes already placed by earlier passes; each entry has matching mesh/location/rotation. */
	TArray<FNProcGenGraphCellNode*> NodeIndex;
	/** Per-cell-node collision hulls, parallel array to NodeIndex. */
	TArray<FNRawMesh> NodeCollisionMeshes;
	/** World-space locations of placed cell nodes, parallel array to NodeIndex. */
	TArray<FVector> NodeCollisionMeshLocations;
	/** World-space rotations of placed cell nodes, parallel array to NodeIndex. */
	TArray<FRotator> NodeCollisionMeshRotations;

	explicit FNVirtualWorldContext(UWorld* TargetWorld, const TArray<FBoxSphereBounds>& TargetBounds)
	{
		InputWorld = TargetWorld;
		InputBounds = TargetBounds;
	}
};
