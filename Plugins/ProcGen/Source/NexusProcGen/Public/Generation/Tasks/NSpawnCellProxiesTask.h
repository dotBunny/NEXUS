// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NSpawnContext.h"

class UNProcGenOperation;

/**
 * Task-graph job that spawns ANCellProxy actors for a slice of pending cell nodes.
 *
 * Runs on the game thread because spawning actors must happen there. The task drains a
 * timesliced batch of nodes from the spawn context per invocation; once the spawn context's
 * cursor reaches the end of its node list, CompletionEvent is signaled to release any
 * downstream tasks that were waiting on the full set of proxies.
 */
struct FNSpawnCellProxiesTask
{
	explicit FNSpawnCellProxiesTask(const TSharedPtr<FNSpawnContext>& SpawnCellsContextPtr,
		const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr,
		const FGraphEventRef& CompletionEvent N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR)
		: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), SpawnCellsContextPtr(SpawnCellsContextPtr.ToSharedRef())
		N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER, CompletionEvent(CompletionEvent)
	{

	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNSpawnCellProxiesTasks, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: spawns proxies for the next timesliced batch of cell nodes. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Top-level task-graph context that receives the spawned proxies. */
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;

	/** Spawn context supplying the cell nodes to process and tracking timeslice progress. */
	TSharedRef<FNSpawnContext> SpawnCellsContextPtr;

	N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF

	/** Event signaled once every cell node has been spawned, gating downstream tasks. */
	FGraphEventRef CompletionEvent;
};
