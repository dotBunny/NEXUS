// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NSpawnContext.h"

class UNProcGenOperation;

struct FNCreateSpawnCellsTask
{
	explicit FNCreateSpawnCellsTask(const TSharedPtr<FNSpawnContext>& SpawnCellsContextPtr, 
		const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr
		N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNCreateSpawnCellsTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: spawns proxies on the game thread from the collected graphs. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	
	/** Top-level task-graph context supplying the graphs and receiving the spawned proxies. */
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;
	TSharedRef<FNSpawnContext> SpawnCellsContextPtr;
	N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF
};
