// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxiesTask.h"


void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
#if !UE_BUILD_SHIPPING	
	const int AnalyticsIndex = AnalyticsPtr->SpawnCellProxiesCreate();
	AnalyticsPtr->SpawnCellProxiesStart(AnalyticsIndex);
#endif // !UE_BUILD_SHIPPING
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
#if !UE_BUILD_SHIPPING	
		AnalyticsPtr->SpawnCellProxiesSpawned(AnalyticsIndex, SpawnCellsContextPtr->Nodes[i]->GetTemplate()->GetFName());
#endif // !UE_BUILD_SHIPPING
		
		if (SpawnCellsContextPtr->CurrentIndex == NodeCount)
		{
#if !UE_BUILD_SHIPPING	
			AnalyticsPtr->SpawnCellProxiesFinish(AnalyticsIndex);
#endif // !UE_BUILD_SHIPPING
			CompletionEvent->Unlock(); // Triggers
			return;
		}

		// Outside of the required slicing, dispatch a new task that should move on to the next frame?
		if (FPlatformTime::Seconds() - StartTime > MaxAllowableTime)
		{
#if !UE_BUILD_SHIPPING	
			AnalyticsPtr->SpawnCellProxiesFinish(AnalyticsIndex);
#endif // !UE_BUILD_SHIPPING
			TGraphTask<FNSpawnCellProxiesTask>::CreateTask(nullptr,  ENamedThreads::GameThread)
				.ConstructAndDispatchWhenReady(SpawnCellsContextPtr, TaskGraphContextPtr, CompletionEvent, AnalyticsPtr);
			return;
		}
		
	}
}
