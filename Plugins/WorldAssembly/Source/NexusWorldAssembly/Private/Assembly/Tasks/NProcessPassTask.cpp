// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NProcessPassTask.h"

FNProcessPassTask::FNProcessPassTask(const TSharedPtr<FNPassContext>& PassContextPtr, 
                                     const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr, 
                                     const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr 
                                     N_ASSEMBLY_ANALYTICS_CONSTRUCTOR) 
:	PassContextPtr(PassContextPtr.ToSharedRef()), WorldContextPtr(WorldContextPtr.ToSharedRef()), 
	TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER
{
	N_ASSEMBLY_ANALYTICS_INDEX_SET(CollectGenerationPassesCreate)
}

void FNProcessPassTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS_INDEX(CollectGenerationPassesStart)
	
	for (TUniquePtr<FNAssemblyGraph>& Graph : PassContextPtr->Graphs)
	{
		// Copy our node collision data to the world
		for (auto& Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
			{
				FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
				WorldContextPtr->NodeIndex.Add(CellNode);
				WorldContextPtr->NodeCollisionMeshes.Add(CellNode->GetHullCopy());
				WorldContextPtr->NodeCollisionMeshLocations.Add(CellNode->GetWorldPosition());
				WorldContextPtr->NodeCollisionMeshRotations.Add(CellNode->GetWorldRotation());
			}
		}
		
		TaskGraphContextPtr->TakeGraph(MoveTemp(Graph));
	}
	TaskGraphContextPtr->AddOutputTags(PassContextPtr->OutputTags);
	PassContextPtr->Graphs.Reset();
	
	N_ASSEMBLY_ANALYTICS_INDEX(CollectGenerationPassesFinish)
}
