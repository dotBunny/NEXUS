// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxiesTask.h"


void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_PROC_GEN_ANALYTICS_INDEX_DEFINE(SpawnCellProxiesCreate)
	N_PROC_GEN_ANALYTICS_INDEX(SpawnCellProxiesStart)
	
	constexpr double MaxAllowableTime = 0.002; // 2ms budget
	const double StartTime = FPlatformTime::Seconds();
	const int NodeCount = SpawnCellsContextPtr->Nodes.Num();
	
	for (int i = SpawnCellsContextPtr->CurrentIndex; i < NodeCount; i++)
	{
		ANCellProxy* Proxy = ANCellProxy::CreateInstance(
		SpawnCellsContextPtr->World, 
		SpawnCellsContextPtr->OperationTicket, 
		SpawnCellsContextPtr->Nodes[i], 
		SpawnCellsContextPtr->bPreloadLevel);
		
		// Registered with global?
		TaskGraphContextPtr->CreatedProxies.Add(Proxy);
			
		// What about creating the instance?
		if (SpawnCellsContextPtr->bSpawnLevelInstance)
		{
			Proxy->CreateLevelInstance();
			Proxy->LoadLevelInstance();
		}
		
		SpawnCellsContextPtr->CurrentIndex++;
		N_PROC_GEN_ANALYTICS_TWO_PARAM(SpawnCellProxiesSpawned, AnalyticsIndex, SpawnCellsContextPtr->Nodes[i]->GetTemplate()->GetFName())
		
		if (SpawnCellsContextPtr->CurrentIndex == NodeCount)
		{
			N_PROC_GEN_ANALYTICS_INDEX(SpawnCellProxiesFinish)
			CompletionEvent->Unlock(); // Triggers
			return;
		}

		// Outside of the required slicing, dispatch a new task that should move on to the next frame?
		if (FPlatformTime::Seconds() - StartTime > MaxAllowableTime)
		{
			N_PROC_GEN_ANALYTICS_INDEX(SpawnCellProxiesFinish)
			TGraphTask<FNSpawnCellProxiesTask>::CreateTask(nullptr,  ENamedThreads::GameThread)
				.ConstructAndDispatchWhenReady(SpawnCellsContextPtr, TaskGraphContextPtr, CompletionEvent N_PROC_GEN_ANALYTICS_CLASS_REF);
			return;
		}
		
	}
}
