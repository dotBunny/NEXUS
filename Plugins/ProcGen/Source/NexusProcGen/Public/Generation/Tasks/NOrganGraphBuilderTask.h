// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NGraphCollectionContext.h"
#include "Generation/Contexts/NOrganContext.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Async/TaskGraphInterfaces.h"

class FNWorldContext;

/**
 * Task-graph job that builds the FNProcGenGraph for a single organ.
 *
 * Runs on a background worker; draws from the per-organ cell/bone input and expands outward
 * from each bone until no open junctions remain or the organ bounds/limits are hit.
 */
struct FNOrganGraphBuilderTask
{
	explicit FNOrganGraphBuilderTask(const TSharedPtr<FNOrganContext>& ContextPtr,
		const TSharedPtr<FNGraphCollectionContext>& PassContextPtr, const TSharedPtr<FNWorldContext>& WorldContextPtr, 
		const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenGraphBuilderTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: performs the full organ build. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	TArray<FNRawMesh> WorldCollisionMeshes;
	TArray<FVector> WorldCollisionLocations;
	TArray<FRotator> WorldCollisionRotations;
	int32 BadStartCount = 0;
	
	
	/** Per-organ input data and output graph reference. */
	TSharedRef<FNOrganContext> ContextPtr;

	/** Shared pass-level context — collects analytics/results across every organ in the step. */
	TSharedRef<FNGraphCollectionContext> PassContextPtr;
	
	TSharedRef<FNProcGenTaskAnalytics> AnalyticsPtr;
	int32 AnalyticsIndex = -1;

	TSharedRef<FNWorldContext> WorldContextPtr;

	
	
	/** Seed the graph with the organ's bones and the root node. */
	void StartGraph(FNMersenneTwister& Random);

	/** @return true when CellNode's hull intersects any of the cached world-collision meshes (parallel mesh/location/rotation arrays). */
	bool DoesWorldCollide(const FNProcGenGraphCellNode* CellNode) const;

	/** @return Every existing cell whose world bounds intersect NewNode's. */
	TArray<FNProcGenGraphCellNode*> CheckNodeBounds(FNProcGenGraphCellNode* NewNode) const;

	/** @return Every existing cell whose hull intersects NewNode's hull. */
	TArray<FNProcGenGraphCellNode*> CheckNodeHull(FNProcGenGraphCellNode* NewNode) const;

	/** Expand the graph from SourceNode, returning the newly-created neighbours. */
	TArray<FNProcGenGraphNode*> ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const;

	/** Cell-node specialization of ProcessNode: picks candidates for each open junction. */
	TArray<FNProcGenGraphNode*> ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const;
};
