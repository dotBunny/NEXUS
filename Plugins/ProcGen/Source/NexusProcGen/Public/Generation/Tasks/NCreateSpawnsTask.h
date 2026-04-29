// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NSpawnContext.h"

class UNProcGenOperation;

/**
 * Task-graph job that flattens every collected per-organ graph into the spawn context's cell-node
 * list, ready for FNSpawnCellProxiesTask to consume.
 *
 * Runs on the game thread so the spawn context can be safely re-built ahead of the spawn passes.
 * Acts as the bridge between the world/pass collection stages and the actual proxy-spawning stage.
 */
struct FNCreateSpawnsTask
{
	explicit FNCreateSpawnsTask(const TSharedPtr<FNSpawnContext>& SpawnCellsContextPtr,
		const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr
		N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNCreateSpawnsTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: flattens the collected graphs into the spawn context. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Top-level task-graph context supplying the graphs and receiving the spawned proxies. */
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;

	/** Spawn context that receives the flattened cell-node list. */
	TSharedRef<FNSpawnContext> SpawnCellsContextPtr;

	N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF
};
