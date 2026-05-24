// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NPassContext.h"
#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Async/TaskGraphInterfaces.h"


class FNVirtualWorldContext;

/**
 * Task-graph job that builds the FNAssemblyGraph for a single organ using the Frontier model.
 *
 * Runs on a background worker; draws from the per-organ cell/bone input and expands outward
 * from each bone until no open junctions remain or the organ bounds/limits are hit.
 */
struct FNOrganGraphBuilderTask
{
	explicit FNOrganGraphBuilderTask(const TSharedPtr<FNVirtualOrganContext>& OrganContextPtr,
		const TSharedPtr<FNPassContext>& PassContextPtr, const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr 
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNWorldAssemblyGraphBuilderTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: performs the full organ build. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Cached world-collision simple-mesh hulls intersecting this organ's bounds. */
	TArray<FNRawMesh> WorldCollisionMeshes;
	/** World-space locations corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FVector> WorldCollisionLocations;
	/** World-space rotations corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FRotator> WorldCollisionRotations;

	/** Cached hulls of cells already placed by earlier organs that overlap this organ's bounds. */
	TArray<FNRawMesh> ExistingNodeCollisionMeshes;
	/** World-space locations corresponding 1:1 with ExistingNodeCollisionMeshes. */
	TArray<FVector> ExistingNodeCollisionLocations;
	/** World-space rotations corresponding 1:1 with ExistingNodeCollisionMeshes. */
	TArray<FRotator> ExistingNodeCollisionRotations;

	/** Number of consecutive failed start-node attempts; used to break out of unwinnable retries. */
	int32 BadStartCount = 0;

	/** Per-organ input data and output graph reference. */
	TSharedRef<FNVirtualOrganContext> OrganContextPtr;

	/** Shared pass-level context. */
	TSharedRef<FNPassContext> PassContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
	N_ASSEMBLY_ANALYTICS_INDEX_LOCAL

	/** Virtual-world context supplying world-collision data for intersection tests. */
	TSharedRef<FNVirtualWorldContext> WorldContextPtr;
	
	/** Seed the graph with the organ's bones and the root node. */
	void StartGraph(FNMersenneTwister& Random);

	/** @return true when CellNode's hull intersects any of the cached world-collision meshes (parallel mesh/location/rotation arrays) and exceeds the penetration threshold. */
	bool DoesWorldCollide(const FNAssemblyGraphCellNode* CellNode) const;
	
	/** @return true when CellNode's hull intersects any of the cached node-collision meshes (parallel mesh/location/rotation arrays) and exceeds the penetration threshold. */
	bool DoesExistingNodeWorldCollide(const FNAssemblyGraphCellNode* CellNode) const;

	/** @return Every existing cell whose world bounds intersect NewNode's. */
	TArray<FNAssemblyGraphCellNode*> CheckNodeBounds(FNAssemblyGraphCellNode* NewNode) const;

	/** @return Every existing cell whose hull intersects NewNode's hull. */
	TArray<FNAssemblyGraphCellNode*> CheckNodeHull(FNAssemblyGraphCellNode* NewNode) const;

	/** Expand the graph from SourceNode, returning the newly-created neighbours. */
	TArray<FNAssemblyGraphNode*> ProcessNode(FNMersenneTwister& Random, FNAssemblyGraphNode* SourceNode) const;

	/** Cell-node specialization of ProcessNode: picks candidates for each open junction. */
	TArray<FNAssemblyGraphNode*> ProcessCellNode(FNMersenneTwister& Random, FNAssemblyGraphCellNode* SourceCellNode) const;
};
