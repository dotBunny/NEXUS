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

	// Cooperative cancellation — drop this pass's graphs instead of propagating them downstream.
	if (TaskGraphContextPtr->IsCancelled())
	{
		PassContextPtr->Graphs.Reset();
		N_ASSEMBLY_ANALYTICS_INDEX(CollectGenerationPassesFinish)
		return;
	}

	TaskGraphContextPtr->SetStatusMessage(FString::Printf(TEXT("Collect Pass (%i)"), PassContextPtr->Graphs.Num()));

	for (TUniquePtr<FNAssemblyGraph>& Graph : PassContextPtr->Graphs)
	{
		// Copy our node collision data to the world
		for (auto& Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
			{
				FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
				WorldContextPtr->NodeIndex.Add(CellNode);
				const int32 HullIndex = WorldContextPtr->NodeCollisionMeshes.Add(CellNode->GetHullCopy());

				// The copy above lands cold — FNRawMesh's copy constructor deliberately drops the face-plane cache
				// and leaves the validation flags dirty. Every organ builder in the following passes reads these
				// same shared entries concurrently through FNOrganGraphBuilderTask::DoesExistingNodeWorldCollide,
				// so leaving them cold means two worker threads can first-touch the same hull and race the lazy
				// writes inside EnsureValidated (four flags) and EnsureCachedFacePlanes (three SetNumUninitialized
				// arrays). Warm both here, on the single thread that owns the array, which is the same treatment
				// FNProcessVirtualWorldTask already applies to the world-collision meshes for this reason.
				//
				// Warmed unconditionally rather than only for convex hulls: the face-plane cache is currently read
				// only on the convex path, but gating on convexity would leave the race latent for any future
				// consumer that touches it on a non-convex hull. The cost is O(loops) per placed cell, once.
				const FNRawMesh& NodeCollisionMesh = WorldContextPtr->NodeCollisionMeshes[HullIndex];
				NodeCollisionMesh.EnsureValidated();
				NodeCollisionMesh.EnsureCachedFacePlanes();
			}
		}

		TaskGraphContextPtr->TakeGraph(MoveTemp(Graph));
	}

	// Next pass access
	WorldContextPtr->ContextTags.AppendTags(PassContextPtr->ContextTags);
	WorldContextPtr->TagCounter.Combine(PassContextPtr->TagCounter);

	// Access on post
	TaskGraphContextPtr->AddContextTags(PassContextPtr->ContextTags);
	TaskGraphContextPtr->AddTagCounter(PassContextPtr->TagCounter);

	PassContextPtr->Graphs.Reset();

	N_ASSEMBLY_ANALYTICS_INDEX(CollectGenerationPassesFinish)
}
