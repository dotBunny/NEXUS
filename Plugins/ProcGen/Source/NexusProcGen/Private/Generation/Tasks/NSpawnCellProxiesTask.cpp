// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxiesTask.h"

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Generation/Graph/NProcGenGraph.h"

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
	
	// Iterate over all graphs that we have had generate
	for (const TUniquePtr<FNProcGenGraph>& Graph : TaskGraphContextPtr->Graphs)
	{
		// Iterate raw nodes of graph and spawn them
		for (const auto Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
			{
				const FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
				
				// Spawn proxy instance
				ANCellProxy* Proxy = ANCellProxy::CreateInstance(TargetWorld, OperationTicket, CellNode, bPreLoadLevelInstances);
		
				// Registered with global?
				TaskGraphContextPtr->CreatedProxies.Add(Proxy);
			
				// What about creating the instance?
				if  (bSpawnLevelInstances)
				{
					Proxy->CreateLevelInstance();
					Proxy->LoadLevelInstance();
				}
			}
		}
	}

#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->SpawnCellProxiesFinish();
#endif // !UE_BUILD_SHIPPING	
}
