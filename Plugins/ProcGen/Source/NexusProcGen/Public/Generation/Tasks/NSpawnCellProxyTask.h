// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"

class UNProcGenOperation;

struct FNSpawnCellProxyTask
{
	explicit FNSpawnCellProxyTask(const TSharedRef<FNProcGenTaskGraphContext>& TaskGraphContextPtr, UWorld* TargetWorld, 
		const TArray<FNProcGenGraphCellNode*>& CellNodes, 
		const uint32 OperationTicket, const bool bPreloadLevel, const bool bSpawnLevelInstance) 
	: TaskGraphContextPtr(TaskGraphContextPtr), Nodes(CellNodes), World(TargetWorld), OperationTicket(OperationTicket),
		bPreloadLevel(bPreloadLevel), bSpawnLevelInstance(bSpawnLevelInstance)
	{
		
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNSpawnCellProxyTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;
	TArray<FNProcGenGraphCellNode*> Nodes;
	UWorld* World;
	uint32 OperationTicket;
	bool bPreloadLevel;
	bool bSpawnLevelInstance;
};
