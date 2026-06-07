// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NSpawnContext.h"

class UNAssemblyOperation;

/**
 * Task-graph job that flattens every collected per-organ graph into the spawn context's cell-node
 * list, ready for FNSpawnCellProxiesTask to consume.
 *
 * Runs on any worker thread (see GetDesiredThread), and multiple operations may run their own instance
 * concurrently, so any shared state it touches must be thread-safe — e.g. it writes the per-operation
 * context through FNWorldAssemblyCache, whose accessors are guarded by a critical section.
 * Acts as the bridge between the world/pass collection stages and the actual proxy-spawning stage.
 */
struct FNCreateSpawnsTask
{
	explicit FNCreateSpawnsTask(const TSharedPtr<FNSpawnContext>& SpawnCellsContextPtr,
		const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNCreateSpawnsTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyNormalThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: flattens the collected graphs into the spawn context. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Top-level task-graph context supplying the graphs and receiving the spawned proxies. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	/** Spawn context that receives the flattened cell-node list. */
	TSharedRef<FNSpawnContext> SpawnCellsContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};
