// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NSpawnCellProxiesTask.h"


void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS_INDEX_DEFINE(SpawnCellProxiesCreate)
	N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesStart)
	
	constexpr double MaxAllowableTime = 0.002; // 2ms budget
	const double StartTime = FPlatformTime::Seconds();
	const int32 NodeCount = SpawnCellsContextPtr->CellNodes.Num();
	
	// Early out when we do not have any nodes to spawn
	if (NodeCount == 0)
	{
		N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesFinish)
		CompletionEvent->Unlock();
		return;
	}
	
	for (int32 i = SpawnCellsContextPtr->CellNodesCurrentIndex; i < NodeCount; i++)
	{
		ANCellProxy* Proxy = ANCellProxy::CreateInstance(
		SpawnCellsContextPtr->World, 
		SpawnCellsContextPtr->OperationTicket, 
		SpawnCellsContextPtr->CellNodes[i], 
		SpawnCellsContextPtr->bPreloadLevels);
		
		// Registered with global?
		TaskGraphContextPtr->CreatedProxies.Add(Proxy);
			
		// What about creating the instance?
		if (SpawnCellsContextPtr->bSpawnLevelInstances)
		{
			Proxy->CreateLevelInstance();
			Proxy->LoadLevelInstance();
		}
		
		SpawnCellsContextPtr->CellNodesCurrentIndex++;
		N_ASSEMBLY_ANALYTICS_TWO_PARAM(SpawnCellProxiesSpawned, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, SpawnCellsContextPtr->CellNodes[i]->GetTemplate()->GetFName())
		
		if (SpawnCellsContextPtr->CellNodesCurrentIndex == NodeCount)
		{
			N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesFinish)
			CompletionEvent->Unlock(); // Triggers
			return;
		}

		// Outside of the required slicing, dispatch a new task that should move on to the next frame?
		if (FPlatformTime::Seconds() - StartTime > MaxAllowableTime)
		{
			N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesFinish)
			TGraphTask<FNSpawnCellProxiesTask>::CreateTask(nullptr,  ENamedThreads::GameThread)
				.ConstructAndDispatchWhenReady(SpawnCellsContextPtr, TaskGraphContextPtr, CompletionEvent N_ASSEMBLY_ANALYTICS_CLASS_REF);
			return;
		}
		
	}
}
