// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NSpawnCellsContext.h"

class UNProcGenOperation;

struct FNSpawnCellProxiesTask
{
	explicit FNSpawnCellProxiesTask(const TSharedPtr<FNSpawnCellsContext>& SpawnCellsContextPtr, 
		const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr,
		const FGraphEventRef& CompletionEvent N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR) 
		: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), SpawnCellsContextPtr(SpawnCellsContextPtr.ToSharedRef()) 
		N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER, CompletionEvent(CompletionEvent)
	{
		
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNSpawnCellProxiesTasks, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;
	TSharedRef<FNSpawnCellsContext> SpawnCellsContextPtr;
	N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF
	FGraphEventRef CompletionEvent;

};
