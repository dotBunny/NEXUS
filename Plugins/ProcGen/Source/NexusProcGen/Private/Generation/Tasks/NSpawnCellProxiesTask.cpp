// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxiesTask.h"

#include "Generation/NProcGenTaskGraphContext.h"
#include "Generation/Graph/NProcGenGraph.h"

FNSpawnCellProxiesTask::FNSpawnCellProxiesTask(const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef())
{
}

void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	const FNProcGenOperationSettings& Settings = TaskGraphContextPtr->OperationSettings;
	
	// Iterate over all graphs that we have had generate
	for (const TUniquePtr<FNProcGenGraph>& Graph : TaskGraphContextPtr->Graphs)
	{
		// Iterate raw nodes of graph and spawn them
		for (const auto Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
			{
				FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
				
				// Spawn proxy instance
				ANCellProxy* Proxy = ANCellProxy::CreateInstance(TaskGraphContextPtr->TargetWorld, CellNode, Settings.bPreLoadLevelInstances);
		
				// Registered with global?
				TaskGraphContextPtr->CreatedProxies.Add(Proxy);
			
				// What about creating the instance?
				if  (Settings.bCreateLevelInstances)
				{
					Proxy->CreateLevelInstance();
				}
			
				// Do we want to load these now?
				if (Settings.bLoadLevelInstances)
				{
					Proxy->LoadLevelInstance();
				}
			}
		}
	}
}
