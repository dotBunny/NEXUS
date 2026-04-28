// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxiesTask.h"


void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{

	
	// constexpr double MaxAllowableTime = 0.01; // 1ms budget
	// const double StartTime = FPlatformTime::Seconds();
	//
	for (int i = SpawnCellsContextPtr->CurrentIndex ; i < SpawnCellsContextPtr->Nodes.Num(); i++)
	{
		ANCellProxy* Proxy = ANCellProxy::CreateInstance(
		SpawnCellsContextPtr->World, 
		SpawnCellsContextPtr->OperationTicket, 
		SpawnCellsContextPtr->Nodes[i], 
		SpawnCellsContextPtr->bPreloadLevel);
		
		// Registered with global?
		TaskGraphContextPtr->CreatedProxies.Add(Proxy);
			
		// What about creating the instance?
		if  (SpawnCellsContextPtr->bSpawnLevelInstance)
		{
			Proxy->CreateLevelInstance();
			Proxy->LoadLevelInstance();
		}
		
		SpawnCellsContextPtr->CurrentIndex++;

		// if (FPlatformTime::Seconds() - StartTime > MaxAllowableTime)
		// {
		// 	TGraphTask<FNSpawnCellProxiesTask>::CreateTask().ConstructAndDispatchWhenReady(SpawnCellsContextPtr, TaskGraphContextPtr);
		// 	return;
		// }
	}
}