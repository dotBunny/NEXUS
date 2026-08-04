// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyOperationSettings.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Math/NBoundsBVH.h"
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

	/**
	 * Broadphase over WorldCollisionMeshes' baked bounds, indexing into that array. Built once by
	 * FNProcessVirtualWorldTask after the meshes are baked, and immutable after — so every organ builder in every
	 * pass shares it without synchronisation, exactly as they share the baked meshes themselves.
	 */
	FNBoundsBVH WorldCollisionBVH;

	/**
	 * Indices into WorldCollisionMeshes whose bounds are invalid, and which therefore cannot be broadphased:
	 * FNRawMeshUtils::GetIntersectDepth skips its AABB rejection when either mesh lacks bounds, so these must be
	 * tested on every candidate to stay equivalent to the original linear scan. Empty for well-formed input.
	 */
	TArray<int32> UnboundedWorldCollisionIndices;

	/** Initial captured transforms before baking */
	TArray<FTransform> WorldCollisionTransforms;

	/** Cell nodes already placed by earlier passes; each entry has matching mesh/location/rotation. */
	TArray<FNAssemblyGraphCellNode*> NodeIndex;

	/**
	 * Per-cell-node collision hulls, parallel array to NodeIndex. Grows between passes only; a builder reads the
	 * prefix it snapshots at task start (see FNVirtualOrganContext::NodeCollisionSnapshotCount), which never mutates
	 * underneath it.
	 */
	TArray<FNRawMesh> NodeCollisionMeshes;

	/**
	 * Swept volumes of the junction connectors accepted so far, as one convex prism per sampled path segment.
	 *
	 * Kept separate from WorldCollisionMeshes rather than appended to it. That array is world-captured geometry
	 * whose BVH is built once and then treated as immutable so every organ builder can share it lock-free; these
	 * prisms are neither world geometry nor available at that point. Kept separate from NodeCollisionMeshes for the
	 * same reason — that array is parallel to NodeIndex, and a connector has no cell node.
	 *
	 * Grown only by FNConnectJunctionsTask, a single task that runs after every organ builder has finished, so
	 * unlike the arrays above this needs no cross-thread discipline.
	 */
	TArray<FNRawMesh> ConnectorCollisionMeshes;

	/** Baked bounds parallel to ConnectorCollisionMeshes, so a probe can reject a prism without touching its geometry. */
	TArray<FBox> ConnectorCollisionBounds;

	/**
	 * Broadphase over the leading ConnectorCollisionIndexedCount entries of ConnectorCollisionBounds.
	 *
	 * Accepted connectors arrive one pairing at a time while the same array is being queried, so rather than
	 * rebuilding per acceptance the tree covers a prefix and the remainder is scanned linearly — the same amortised
	 * arrangement FNAssemblyGraph uses for its cell-node index, and for the same reason.
	 */
	FNBoundsBVH ConnectorCollisionBVH;

	/** How many leading entries of ConnectorCollisionBounds the tree covers; the rest are scanned. */
	int32 ConnectorCollisionIndexedCount = 0;

	/**
	 * Rebuild ConnectorCollisionBVH once the unindexed tail exceeds this many prisms. Bounds the linear part of a
	 * query while keeping rebuilds amortised, matching FNAssemblyGraph::CellNodeIndexTailThreshold.
	 */
	static constexpr int32 ConnectorCollisionIndexTailThreshold = 64;

	/** Bring ConnectorCollisionBVH up to date when the unindexed tail has grown past the threshold. */
	void EnsureConnectorCollisionIndex()
	{
		const int32 MeshCount = ConnectorCollisionBounds.Num();
		if (MeshCount - ConnectorCollisionIndexedCount <= ConnectorCollisionIndexTailThreshold)
		{
			return;
		}

		ConnectorCollisionBVH = FNBoundsBVH(ConnectorCollisionBounds);
		ConnectorCollisionIndexedCount = MeshCount;
	}

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
