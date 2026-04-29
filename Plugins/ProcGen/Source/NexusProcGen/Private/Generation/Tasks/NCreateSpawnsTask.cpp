// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NCreateSpawnsTask.h"

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Generation/Graph/NProcGenGraph.h"

FNCreateSpawnsTask::FNCreateSpawnsTask(
	const TSharedPtr<FNSpawnContext>& SpawnCellsContextPtr, 
	const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), SpawnCellsContextPtr(SpawnCellsContextPtr.ToSharedRef()) N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER
{
}

void FNCreateSpawnsTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_PROCEDURAL_GENERATION_ANALYTICS(CreateSpawnCellsContextStart)
	
	// Iterate over all graphs that we have had generate
	for (const TUniquePtr<FNProcGenGraph>& Graph : TaskGraphContextPtr->Graphs)
	{
		// Iterate raw nodes of graph and spawn them
		for (const auto Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
			{
				// Add to our list of things to spawn
				FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
				SpawnCellsContextPtr->CellNodes.Add(CellNode);
			}
		}
	}

	N_PROCEDURAL_GENERATION_ANALYTICS(CreateSpawnCellsContextFinish)
}