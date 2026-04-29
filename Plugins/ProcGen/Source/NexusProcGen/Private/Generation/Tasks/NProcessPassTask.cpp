// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcessPassTask.h"

FNProcessPassTask::FNProcessPassTask(const TSharedPtr<FNPassContext>& PassContextPtr, 
                                     const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr, 
                                     const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr 
                                     N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR) 
:	PassContextPtr(PassContextPtr.ToSharedRef()), WorldContextPtr(WorldContextPtr.ToSharedRef()), 
	TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER
{
	N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_SET(CollectGenerationPassesCreate)
}

void FNProcessPassTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_PROCEDURAL_GENERATION_ANALYTICS_INDEX(CollectGenerationPassesStart)
	
	for (TUniquePtr<FNProcGenGraph>& Graph : PassContextPtr->Graphs)
	{
		// Copy our node collision data to the world
		for (auto& Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENProcGenGraphNodeType::Cell)
			{
				FNProcGenGraphCellNode* CellNode = static_cast<FNProcGenGraphCellNode*>(Node);
				WorldContextPtr->NodeIndex.Add(CellNode);
				WorldContextPtr->NodeCollisionMeshes.Add(CellNode->GetHullCopy());
				WorldContextPtr->NodeCollisionMeshLocations.Add(CellNode->GetWorldPosition());
				WorldContextPtr->NodeCollisionMeshRotations.Add(CellNode->GetWorldRotation());
			}
		}
		
		TaskGraphContextPtr->TakeGraph(MoveTemp(Graph));
	}
	PassContextPtr->Graphs.Reset();
	
	N_PROCEDURAL_GENERATION_ANALYTICS_INDEX(CollectGenerationPassesFinish)
}
