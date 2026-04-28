// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxiesTask.h"

#include "NProcGenMinimal.h"
#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Generation/Graph/NProcGenGraph.h"
#include "Generation/Tasks/NSpawnCellProxyTask.h"

FNSpawnCellProxiesTask::FNSpawnCellProxiesTask(const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr,
                                               const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), AnalyticsPtr(AnalyticsPtr.ToSharedRef())
{
}

void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->SpawnCellProxiesStart();
#endif // !UE_BUILD_SHIPPING	
	
	const FNProcGenOperationSettings& Settings = TaskGraphContextPtr->OperationSettings;
	const bool bPreLoadLevelInstances = Settings.bPreLoadLevelInstances;
	const bool bSpawnLevelInstances  = Settings.bCreateLevelInstances;
	UWorld* TargetWorld = TaskGraphContextPtr->TargetWorld;
	const uint32 OperationTicket = TaskGraphContextPtr->OperationTicket;
	
	// TODO: Change this to a global config value?
	int CellsPerTask = 1;
	TArray<FNProcGenGraphCellNode*> CellNodes;
	FGraphEventArray SpawnCellProxyTasks;
	FGraphEventArray LastTask;
	
	
	// Iterate over all graphs that we have had generate
	for (const TUniquePtr<FNProcGenGraph>& Graph : TaskGraphContextPtr->Graphs)
	{
		// TODO: Should track the graph its attached too for analytics?
		
		// Iterate raw nodes of graph and spawn them
		for (const auto Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
			{
				
				FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
				CellNodes.Add(CellNode);
				
				if (CellNodes.Num() >= CellsPerTask)
				{
					const auto Task = 
				TGraphTask<FNSpawnCellProxyTask>::CreateTask(
					(LastTask.Num() > 0) ? &LastTask : nullptr,
					ENamedThreads::GameThread)
					.ConstructAndHold(TaskGraphContextPtr, TargetWorld, CellNodes, OperationTicket, 
						bPreLoadLevelInstances, bSpawnLevelInstances);
					
					// Add to overall list of proxy tasks
					SpawnCellProxyTasks.Add(Task);
					
					// Cache for future dependency
					LastTask = FGraphEventArray();
					LastTask.Add(Task);
					
					CellNodes.Empty();
				}
			}
		}
		
		if (CellNodes.Num() > 0)
		{
			const auto Task = TGraphTask<FNSpawnCellProxyTask>::CreateTask(
			(LastTask.Num() > 0) ? &LastTask : nullptr,
			ENamedThreads::GameThread)
			.ConstructAndHold(TaskGraphContextPtr, TargetWorld, CellNodes, OperationTicket, 
				bPreLoadLevelInstances, bSpawnLevelInstances);
			
			// Add to overall list of proxy tasks
			SpawnCellProxyTasks.Add(Task);
					
			// Cache for future dependency
			LastTask = FGraphEventArray();
			LastTask.Add(Task);
					
			CellNodes.Empty();
		}
	}
	

	// TODO: try and come up with a away to spread these across frames, right now they depend on each other maybe that helps
	// Limit per frame running
	int CurrentTaskIndex = 0;
	while (CurrentTaskIndex < SpawnCellProxyTasks.Num())
	{
		const uint64 CurrentFrameNumber = GFrameCounter;
		const FGraphEventRef TaskRef = SpawnCellProxyTasks[CurrentTaskIndex];
		
		TaskRef->Unlock();
		//FPlatformProcess::Sleep(0.1f);
		//
		// // Wait till next frame
		// do
		// {
		// 	//FPlatformProcess::Sleep(0.00001f);
		// }
		// while (CurrentFrameNumber == GFrameCounter);
		
		CurrentTaskIndex++;
	}

#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->SpawnCellProxiesFinish();
#endif // !UE_BUILD_SHIPPING	
}