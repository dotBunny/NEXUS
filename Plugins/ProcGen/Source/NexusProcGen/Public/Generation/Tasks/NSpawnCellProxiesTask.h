// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/NProcGenTaskAnalytics.h"

class UNProcGenOperation;

struct FNSpawnCellProxiesTask
{
	explicit FNSpawnCellProxiesTask(const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr,
		const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenCreateProxiesTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: spawns proxies on the game thread from the collected graphs. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	
	/** Top-level task-graph context supplying the graphs and receiving the spawned proxies. */
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;
	
	TSharedRef<FNProcGenTaskAnalytics> AnalyticsPtr;
};
