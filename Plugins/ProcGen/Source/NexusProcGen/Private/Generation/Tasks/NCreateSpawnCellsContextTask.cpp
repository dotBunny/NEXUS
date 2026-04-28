// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NCreateSpawnCellsContextTask.h"

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Generation/Graph/NProcGenGraph.h"

FNCreateSpawnCellsContextTask::FNCreateSpawnCellsContextTask(
	const TSharedPtr<FNSpawnCellsContext>& SpawnCellsContextPtr, 
	const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr,
	const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), SpawnCellsContextPtr(SpawnCellsContextPtr.ToSharedRef()), AnalyticsPtr(AnalyticsPtr.ToSharedRef())
{
}

void FNCreateSpawnCellsContextTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->CreateSpawnCellsContextStart();
#endif // !UE_BUILD_SHIPPING	
	
	// Iterate over all graphs that we have had generate
	for (const TUniquePtr<FNProcGenGraph>& Graph : TaskGraphContextPtr->Graphs)
	{
		// TODO: Should track the graph its attached too for analytics?
		
		// Iterate raw nodes of graph and spawn them
		for (const auto Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
			{
				// Add to our list of things to spawn
				FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
				SpawnCellsContextPtr->Nodes.Add(CellNode);
			}
		}
	}
	
#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->CreateSpawnCellsContextFinish();
#endif // !UE_BUILD_SHIPPING	
}