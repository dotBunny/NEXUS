// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NCreateSpawnsTask.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyContextCache.h"
#include "NWorldAssemblyRegistry.h"
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

	// We need to prepopulate some data elsewhere
	FNWorldAssemblyContextCache::AddOperationContext(TaskGraphContextPtr->OperationTicket, TaskGraphContextPtr->TagCounter, TaskGraphContextPtr->ContextTags);

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
				
				// Build out the data we are going to use
				CellNode->GenerateLinkDetails();
				
				SpawnCellsContextPtr->CellNodes.Add(CellNode);
			}
		}
	}

	// The spawn list is now complete, so we can report the full cell count as proxy spawning begins.
	TaskGraphContextPtr->SetStatusMessage(FString::Printf(TEXT("%s (%i)"),
		*NEXUS::WorldAssembly::StatusMessage::SpawningCells, SpawnCellsContextPtr->CellNodes.Num()));

	N_ASSEMBLY_ANALYTICS(CreateSpawnCellsContextFinish)
}