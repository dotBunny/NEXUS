// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NProcGenOperationSharedContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/Graph/NProcGenGraph.h"

class UNProcGenOperation;

struct FNProcGenCreateProxiesTask
{
	explicit FNProcGenCreateProxiesTask(TUniquePtr<FNProcGenGraph> CellGraph, TSharedRef<FNProcGenOperationSharedContext> SharedContext);
	~FNProcGenCreateProxiesTask();
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenCreateProxiesTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	TUniquePtr<FNProcGenGraph> CellGraph;
};
