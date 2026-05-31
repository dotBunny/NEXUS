// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NCreateSpawnsTask.h"

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/Graph/NAssemblyGraph.h"

FNCreateSpawnsTask::FNCreateSpawnsTask(
	const TSharedPtr<FNSpawnContext>& SpawnCellsContextPtr, 
	const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), SpawnCellsContextPtr(SpawnCellsContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER
{
}

void FNCreateSpawnsTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Early out if we've already canceled it.
	if (SpawnCellsContextPtr->bCancelled.Load())
	{
		CompletionGraphEvent->Unlock();
		return;
	}
		
	N_ASSEMBLY_ANALYTICS(CreateSpawnCellsContextStart)
	
	// Iterate over all graphs that we have had generate
	for (const TUniquePtr<FNAssemblyGraph>& Graph : TaskGraphContextPtr->Graphs)
	{
		// Iterate raw nodes of graph and spawn them
		for (const auto Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
			{
				// Add to our list of things to spawn
				FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
				SpawnCellsContextPtr->CellNodes.Add(CellNode);
			}
		}
	}

	N_ASSEMBLY_ANALYTICS(CreateSpawnCellsContextFinish)
}