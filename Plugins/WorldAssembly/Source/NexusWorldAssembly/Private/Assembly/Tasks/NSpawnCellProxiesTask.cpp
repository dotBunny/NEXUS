// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NSpawnCellProxiesTask.h"


void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS_INDEX_DEFINE(SpawnCellProxiesCreate)
	N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesStart)
	
	// Cancellation check — the operation may have been cancelled between timeslice dispatches
	if (SpawnCellsContextPtr->bCancelled.Load())
	{
		CompletionEvent->Unlock();
		return;
	}

	// Dispatch Guard #1
	UWorld* TargetWorld = SpawnCellsContextPtr->World;
	if (!IsValid(TargetWorld) || TargetWorld->bIsTearingDown)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Skipping dispatched FNSpawnCellProxiesTask as the World is in a bad state (PIE, teardown, etc.). Any associated reports SpawnCellProxies timers will be incorrect."));
		CompletionEvent->Unlock();
		return;
	}
	
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
		
		// Dispatch Guard #2
		if (Proxy == nullptr)
		{
			// SpawnActor failed (e.g. world began tearing down mid-batch); abandon remaining nodes.
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Bailing on FNSpawnCellProxiesTask as it was not able to spawn a proxy, most likely cause the World is in a bad state. Any associated reports SpawnCellProxies timers will be incorrect."));
			CompletionEvent->Unlock();
			return;
		}
		
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
