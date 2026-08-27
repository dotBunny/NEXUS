// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Async/TaskGraphInterfaces.h"

/**
 * Task-graph job that derives per-cell data from the finished graphs, before anything is turned into a spawn.
 *
 * Runs three passes, in this order and for a reason. Hot path resolution flags the cells lying on each of the two
 * routes through the assembly. Proximity scoring then measures every cell's distance from those routes and from the
 * Important-flagged cells. Link-detail generation last, emitting the per-junction connection record each cell
 * carries to runtime.
 *
 * The ordering is load-bearing across the whole operation, not just within one graph: the junction-connector pass
 * links cells that can belong to *different* graphs, so a link detail generated before the far graph was flagged
 * would bake in a stale false, and a proximity score resolved per-graph would report a cell one connector from a
 * neighbouring organ's landmark as unreachable. Each pass therefore completes for every graph before the next
 * begins, which is why they sit in one task rather than being folded into the per-graph work upstream.
 *
 * Sits between FNConnectJunctionsTask — which must have finished creating cross-graph links for the above to hold —
 * and FNCreateSpawnsTask, which only flattens the result into the spawn list.
 *
 * Runs on any worker thread (see GetDesiredThread). By the time its prerequisites have completed the graphs are no
 * longer being mutated, and it is the only writer of the flags, scores and link details it produces.
 */
struct FNEvaluateGraphsTask
{
	/**
	 * @param TaskGraphContextPtr Task-graph context supplying the built graphs. The only context this stage needs —
	 *        it neither reads nor writes the spawn context, including for cancellation, which it takes from here.
	 * @note Exported, unlike its sibling tasks, so the test module can drive DoTask directly. The three passes are
	 *       covered in depth against FNAssemblyGraph, but every one of those tests stays green if this task stops
	 *       calling one of them — that only shows up as cells reaching runtime with default data.
	 */
	NEXUSWORLDASSEMBLY_API explicit FNEvaluateGraphsTask(const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNEvaluateGraphsTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyNormalThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: resolves hot paths, scores proximity, and generates link details. */
	NEXUSWORLDASSEMBLY_API void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Top-level task-graph context supplying the graphs this stage evaluates. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};
