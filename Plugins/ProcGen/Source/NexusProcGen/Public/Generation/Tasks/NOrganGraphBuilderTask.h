// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCollectPassContext.h"
#include "Generation/Tasks/NOrganGraphBuilderContext.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Async/TaskGraphInterfaces.h"

/**
 * Task-graph job that builds the FNProcGenGraph for a single organ.
 *
 * Runs on a background worker; draws from the per-organ cell/bone input and expands outward
 * from each bone until no open junctions remain or the organ bounds/limits are hit.
 */
struct FNOrganGraphBuilderTask
{
	explicit FNOrganGraphBuilderTask(const TSharedPtr<FNOrganGraphBuilderContext>& ContextPtr,
		const TSharedPtr<FNCollectPassContext>& PassContextPtr);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenGraphBuilderTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: performs the full organ build. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:

	/** Per-organ input data and output graph reference. */
	TSharedRef<FNOrganGraphBuilderContext> ContextPtr;

	/** Shared pass-level context — collects analytics/results across every organ in the step. */
	TSharedRef<FNCollectPassContext> PassContextPtr;

	/** Analytics recorded while this task runs; forwarded to the pass context on completion. */
	mutable FNOrganGraphBuilderAnalytics Analytics;

	/** Seed the graph with the organ's bones and the root node. */
	void StartGraph(FNMersenneTwister& Random) const;

	/** @return Every existing cell whose world bounds intersect NewNode's. */
	TArray<FNProcGenGraphCellNode*> CheckNodeBounds(FNProcGenGraphCellNode* NewNode) const;

	/** @return Every existing cell whose hull intersects NewNode's hull. */
	TArray<FNProcGenGraphCellNode*> CheckNodeHull(FNProcGenGraphCellNode* NewNode) const;

	/** Expand the graph from SourceNode, returning the newly-created neighbours. */
	TArray<FNProcGenGraphNode*> ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const;

	/** Cell-node specialization of ProcessNode: picks candidates for each open junction. */
	TArray<FNProcGenGraphNode*> ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const;
};
