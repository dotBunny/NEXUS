// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NOrganGraphBuilderTask.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyGameplayTags.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Assembly/Graph/NAssemblyGraphNullNode.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Math/NMersenneTwister.h"

FNOrganGraphBuilderTask::FNOrganGraphBuilderTask(const TSharedPtr<FNVirtualOrganContext>& OrganContextPtr,
	const TSharedPtr<FNPassContext>& PassContextPtr, const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr,
	const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	:	OrganContextPtr(OrganContextPtr.ToSharedRef()), PassContextPtr(PassContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER,
		WorldContextPtr(WorldContextPtr.ToSharedRef()), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef())
{
	N_ASSEMBLY_ANALYTICS_INDEX_SET(OrganGraphBuilderCreate)
}

void FNOrganGraphBuilderTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Open a per-organ status channel so this build surfaces as its own child progress bar in the overlay.
	const int32 StatusChannel = TaskGraphContextPtr->OpenStatusChannel(OrganContextPtr->GetName());

	N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderStart)
	N_ASSEMBLY_ANALYTICS_FIVE_PARAM(OrganGraphBuilder_Init, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, OrganContextPtr->GetName(), 
		OrganContextPtr->MinimumCellCount, OrganContextPtr->MaximumCellCount, OrganContextPtr->MaximumRetryCount)
	
	// The context was not validated during creation, so we cannot process it
	if (!OrganContextPtr->IsValid())
	{
		N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetFailure, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, 0,
			FString(TEXT("Organ context failed validation; build was not attempted.")))
		N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderFinish)
		// Drive the per-organ channel to a terminal state so its overlay row doesn't linger mid-build.
		TaskGraphContextPtr->SetChannelStatus(StatusChannel, TEXT("Unsuccessful"), 0.f);
		TaskGraphContextPtr->CloseStatusChannel(StatusChannel);
		return;
	}
	
	// Cooperative cancellation — the operation may have been cancelled before this organ's build started.
	if (TaskGraphContextPtr->IsCancelled())
	{
		N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetFailure, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, 0,
			FString(TEXT("Operation was cancelled before the organ build started.")))
		N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderFinish)
		// Drive the per-organ channel to a terminal state so its overlay row doesn't linger mid-build.
		TaskGraphContextPtr->SetChannelStatus(StatusChannel, TEXT("Cancelled"), 0.f);
		TaskGraphContextPtr->CloseStatusChannel(StatusChannel);
		return;
	}

	// Create our deterministic random for the task which will get passed byref to sub-methods
	FNMersenneTwister Random(OrganContextPtr->GetSeed());
	
	// World-collision hulls are baked (and face-plane cached) by FNProcessVirtualWorldTask and immutable after, so they
	// are read straight from the shared context instead of copied per organ (FNRawMesh copies drop the face-plane cache).
	// The node hulls grow between passes, so snapshot how many exist now; entries below the count never mutate, and
	// FNProcessPassTask's appends never overlap a builder thanks to the pass dependency chain.
	ExistingNodeCollisionMeshCount = WorldContextPtr->NodeCollisionMeshes.Num();
	
	// Capture our context tags, base that we cant avoid, and our working copy
	OrganContextPtr->BaseContextTags = WorldContextPtr->ContextTags;
	OrganContextPtr->BaseTagCounter = WorldContextPtr->TagCounter;
	OrganContextPtr->TagCounter = OrganContextPtr->BaseTagCounter;
	OrganContextPtr->ContextTags = OrganContextPtr->BaseContextTags;
	
	while (!OrganContextPtr->IsSuccessful())
	{
		// Cooperative cancellation — stop retrying and fall through to the unsuccessful teardown below.
		if (TaskGraphContextPtr->IsCancelled())
		{
			break;
		}

		TaskGraphContextPtr->SetChannelStatus(StatusChannel, FString::Printf(TEXT("Iteration %i/%i"), OrganContextPtr->GetRetryCount(), OrganContextPtr->MaximumRetryCount));
		
		// Coarse attempt-based progress for the per-organ channel; success drives it to 100% below.
		const int32 MaxRetries = FMath::Max(OrganContextPtr->MaximumRetryCount, 1);
		TaskGraphContextPtr->SetChannelStatus(StatusChannel,
			FString::Printf(TEXT("Attempt %i/%i"), OrganContextPtr->GetRetryCount(), OrganContextPtr->MaximumRetryCount),
			static_cast<float>(OrganContextPtr->GetRetryCount()) / MaxRetries);

		// Find the bone and build our starting cell
		StartGraph(Random);
	
		// Check for start placement and that it was a node too
		if (OrganContextPtr->CellGraph == nullptr)
		{
			N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetFailure, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, 0,
				FString(TEXT("Unable to place a starting cell (no valid starter cell available or every candidate collided).")))
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderFinish)
			// Drive the per-organ channel to a terminal state so its overlay row doesn't linger mid-build.
			TaskGraphContextPtr->SetChannelStatus(StatusChannel, TEXT("Unsuccessful"), 0.f);
			TaskGraphContextPtr->CloseStatusChannel(StatusChannel);
			return;
		}
		int32 NodeCount = OrganContextPtr->CellGraph->GetNodeCount();
		if (NodeCount == 0)
		{
			N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetFailure, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, 0,
				FString(TEXT("Starting graph contained no nodes after placement.")))
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderFinish)
			// Drive the per-organ channel to a terminal state so its overlay row doesn't linger mid-build.
			TaskGraphContextPtr->SetChannelStatus(StatusChannel, TEXT("Unsuccessful"), 0.f);
			TaskGraphContextPtr->CloseStatusChannel(StatusChannel);
			return;
		}

		// Break before this could have been bad
		if (OrganContextPtr->CellGraph->GetNodesWithOpenJunctions().IsEmpty())
		{
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("The starter cell has no open junctions. This is a BAD cell to have as a starter. Retrying ..."));
			N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetFailure, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX,
				OrganContextPtr->CellGraph->GetCellNodeCount(),
				FString(TEXT("Starter cell has no open junctions; the graph cannot grow past it.")))
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderFinish)
			// Drive the per-organ channel to a terminal state so its overlay row doesn't linger mid-build.
			TaskGraphContextPtr->SetChannelStatus(StatusChannel, TEXT("Unsuccessful"), 0.f);
			TaskGraphContextPtr->CloseStatusChannel(StatusChannel);
			return;
		}
		
		const int32 MinCellCount = FMath::Max(OrganContextPtr->MinimumCellCount, 1);
		const int32 MaxCellCount = OrganContextPtr->MaximumCellCount;
		// MaximumCellCount <= 0 means "no upper limit": BFS expands unbounded and the target degrades to a floor
		// (MinCellCount) for the secondary fill loop. Made explicit here because IntegerRange(min, -1) silently
		// clamps to min, which reads like a bug.
		const int32 TargetCellCount = (MaxCellCount > MinCellCount)
			? Random.IntegerRange(MinCellCount, MaxCellCount)
			: MinCellCount;

		// BFS expansion: grow the graph wave-by-wave, stopping when we reach the maximum
		TArray<FNAssemblyGraphNode*> Frontier = ProcessNode(Random, OrganContextPtr->CellGraph->GetLastNode());
		while (Frontier.Num() > 0)
		{
			if (TaskGraphContextPtr->IsCancelled())
			{
				break;
			}
			if (MaxCellCount > 0 && OrganContextPtr->CellGraph->GetCellNodeCount() >= MaxCellCount)
			{
				break;
			}

			TArray<FNAssemblyGraphNode*> NextFrontier;
			for (int32 j = 0; j < Frontier.Num(); j++)
			{
				if (MaxCellCount > 0 && OrganContextPtr->CellGraph->GetCellNodeCount() >= MaxCellCount)
				{
					break;
				}
				NextFrontier.Append(ProcessNode(Random, Frontier[j]));
			}
			Frontier = MoveTemp(NextFrontier);
		}

		// If still below minimum, try filling remaining open junctions
		while (OrganContextPtr->CellGraph->GetCellNodeCount() < TargetCellCount)
		{
			if (TaskGraphContextPtr->IsCancelled()) break;

			TArray<FNAssemblyGraphNode*> OpenNodes = OrganContextPtr->CellGraph->GetNodesWithOpenJunctions();
			if (OpenNodes.IsEmpty()) break;

			const int32 CountBefore = OrganContextPtr->CellGraph->GetCellNodeCount();
			for (int32 j = 0; j < OpenNodes.Num(); j++)
			{
				if (OrganContextPtr->CellGraph->GetCellNodeCount() >= TargetCellCount) break;
				ProcessNode(Random, OpenNodes[j]);
			}

			if (OrganContextPtr->CellGraph->GetCellNodeCount() == CountBefore) break;
		}

		// Cooperative cancellation — skip the finishing passes and validation; the result will not be used.
		if (TaskGraphContextPtr->IsCancelled())
		{
			break;
		}

		// Guarantee finisher-eligible cells reach their MinimumCount before the opportunistic cap below claims the
		// remaining open junctions. No-op unless such a cell is actually short, so non-finisher tissues are unchanged.
		EnsureFinisherMinimums(Random);

		CapBranchesWithFinishers(Random);

		EnforceNotFinisherConstraint();

		// At this point we need to validate the graph against the overall requirements from the input settings.
		// The bSuccessful flag gets set at this point, but if it isn't valid we then need to take our chances and regenerate,
		// but only if we have not run out of attempts.
		if (!OrganContextPtr->ValidateGraph())
		{
			N_ASSEMBLY_ANALYTICS_TWO_PARAM(OrganGraphBuilder_AddMessages, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, OrganContextPtr->GetMessages())
			// Record the failing verdict before ResetForRetry drops the graph; last write wins if a later attempt passes.
			N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetResult, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, false, OrganContextPtr->CellGraph->GetCellNodeCount())

			if (!OrganContextPtr->ResetForRetry())
			{
				// We cant retry we're going to have to break
				break;
			}

			// Prepare analytics for another pass
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_NextIteration)
		}
		else
		{
			N_ASSEMBLY_ANALYTICS_TWO_PARAM(OrganGraphBuilder_AddMessages, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, OrganContextPtr->GetMessages())
			N_ASSEMBLY_ANALYTICS_THREE_PARAM(OrganGraphBuilder_SetResult, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, true, OrganContextPtr->CellGraph->GetCellNodeCount())
		}
	}
	
	// Clean up graph of pointers that can't be kept
	if (OrganContextPtr->CellGraph != nullptr)
	{
		OrganContextPtr->CellGraph->CleanupBuilderReferences();
	}
	
	// Only hand off graph and the context tag collection if it's good
	// TODO: Would we do something here if we wanted required to apply ACROSS organs?
	if (OrganContextPtr->IsSuccessful())
	{
		// Mark the per-organ channel complete before CellGraph is moved out below.
		TaskGraphContextPtr->SetChannelStatus(StatusChannel,
			FString::Printf(TEXT("Built (%i cells)"), OrganContextPtr->CellGraph->GetCellNodeCount()), 1.f);
		TaskGraphContextPtr->CloseStatusChannel(StatusChannel);

		// Report findings for later
		TaskGraphContextPtr->SetOrganCellCount(OrganContextPtr->GetIdentifier(), OrganContextPtr->CellGraph->GetCellNodeCount());
		
		// Hand off only this organ's contribution. The working TagCounter was seeded with BaseTagCounter so
		// constraints could gate against absolute counts, but the base already lives in the world/task-graph
		// counter the pass Combines into; differencing it out leaves just this organ's delta to avoid
		// multi-counting the base. ContextTags needs no such treatment as its fan-in is an idempotent union.
		PassContextPtr->TakeOutput(
			MoveTemp(OrganContextPtr->CellGraph),
			OrganContextPtr->ContextTags,
			OrganContextPtr->TagCounter.GetDifference(OrganContextPtr->BaseTagCounter));
	}
	else
	{
		// Ensure we close the channel
		if (TaskGraphContextPtr->IsCancelled())
		{
			TaskGraphContextPtr->SetChannelStatus(StatusChannel,TEXT("Cancelled"), 0.f);
		}
		else if (OrganContextPtr->IsRequired())
		{
			TaskGraphContextPtr->SetChannelStatus(StatusChannel,TEXT("Unsuccessful"), 0.f);
		}
		else
		{
			TaskGraphContextPtr->SetChannelStatus(StatusChannel,TEXT("Unsuccessful (Not Required)"), 1.f);
		}
		TaskGraphContextPtr->SetOrganCellCount(OrganContextPtr->GetIdentifier(), 0);
		TaskGraphContextPtr->CloseStatusChannel(StatusChannel);
	}

	N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilderFinish)
}

void FNOrganGraphBuilderTask::StartGraph(FNMersenneTwister& Random)
{
	// TODO: We haven't resolved yet how we might join multiple generation points yet so we are just going to use the first bone.
	FNVirtualBoneData& BoneData = OrganContextPtr->BoneInputData[0];
	
	FNCellInputDataFilter PreFilter;
	PreFilter.NodeDepth = 0;
	PreFilter.SocketSize = BoneData.SocketSize;
	PreFilter.SourceQuat = FQuat(BoneData.WorldRotation);
	PreFilter.bIsStartNode = true;
	PreFilter.WorldPosition = BoneData.WorldPosition;

	FNWeightedIntegerArray WeightedStartIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	OrganContextPtr->FilterCellInputData(PreFilter, WeightedStartIndices, ValidJunctions);
	
	// Unable to generate
	if (WeightedStartIndices.WeightedCount() == 0)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to place starting cell, due to no valid cells available."));
		return;
	}
	
	
	// Our starting placement has to happen
	do
	{
		// Pull a candidate starting cell and remove it so it can never be re-rolled. Because we try every
		// one of its junctions below, removing the cell here cannot discard a junction we have not evaluated.
		const int32 StartCellIndex = WeightedStartIndices.TwistedValueAndRemove(Random);
		if (StartCellIndex == INDEX_NONE)
		{
			// Every candidate starting cell has been exhausted without a successful placement.
			break;
		}
		FNVirtualCellData* StartCellInputData = &OrganContextPtr->CellInputData[StartCellIndex];

		// Try this cell's appropriately sized junctions in a random order, without replacement, until one
		// places. The cell is already removed above, so draining its junction list here is safe.
		TArray<int32>& ValidJunctionIndices = ValidJunctions[StartCellIndex];
		while (ValidJunctionIndices.Num() > 0)
		{
			const int32 TargetJunctionKeyIndex = Random.IntegerRange(0, ValidJunctionIndices.Num()-1);
			const int32 TargetJunctionKey = ValidJunctionIndices[TargetJunctionKeyIndex];
			ValidJunctionIndices.RemoveAtSwap(TargetJunctionKeyIndex, EAllowShrinking::No);

			// Source guarantees this will find something.
			const FNCellJunctionDetails* StartCellJunctionDetails = StartCellInputData->Junctions.Find(TargetJunctionKey);

			// When matching to a Bone, we want to find the rotation necessary to match the Bone's facing direction (forward) to the Junctions facing
			// direction (forward). This is not the common-case when we match a junction to a junction, in that case we want the opposite facing directions.
			// Since the StartGraph method is connecting a bone to a junction, we match their facing directions.
			const FQuat BoneQuat = BoneData.WorldRotation.Quaternion();
			const FQuat JunctionQuat = StartCellJunctionDetails->WorldRotation.Quaternion();

			const FQuat CellWorldQuat = BoneQuat * JunctionQuat.Inverse();
			const FRotator CellWorldRotation = CellWorldQuat.Rotator();
			const FVector JunctionWorldOffset = CellWorldQuat.RotateVector(StartCellJunctionDetails->WorldLocation);
			const FVector CellWorldPosition = BoneData.WorldPosition - JunctionWorldOffset;

			// Create our bone node and build a graph around it.
			FNAssemblyGraphBoneNode* BoneNode = FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, CellWorldPosition, CellWorldRotation);

			// Create our graph
			OrganContextPtr->CellGraph = MakeUnique<FNAssemblyGraph>(
				BoneNode, OrganContextPtr->Origin, OrganContextPtr->Bounds, OrganContextPtr->bUnbound);


			// Create our first cell node, attaching it to the bone node
			FNAssemblyGraphNodeParams NodeParams;
			NodeParams.ContextTagsAdded = StartCellInputData->ContextTagsAdded;
			NodeParams.AssemblyTags = StartCellInputData->AssemblyTags;
			NodeParams.Seed = Random.UnsignedInteger64();
			NodeParams.WorldPosition = CellWorldPosition;
			NodeParams.WorldRotation = CellWorldRotation;
			FNAssemblyGraphCellNode* StartNode = FNAssemblyGraphNodeFactory::CreateCellNode(NodeParams, StartCellInputData, OrganContextPtr->VoxelSize);
			
			
			if (DoesWorldCollide(StartNode))
			{
				delete StartNode;
				OrganContextPtr->CellGraph.Reset(); // Bone is already part of graph
				N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardWorldCollidingStart)
			}
			else if (DoesExistingNodeWorldCollide(StartNode))
			{
				delete StartNode;
				OrganContextPtr->CellGraph.Reset(); // Bone is already part of graph
				N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardExistingNodeWorldCollidingCellNode)
			}
			else
			{
				OrganContextPtr->CellGraph->RegisterNode(StartNode);

				// Our starting cell has unique tags that need to get added to the used
				if (StartCellInputData->AssemblyTags.HasAnyExact(OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags))
				{
					OrganContextPtr->PlacedTagGroups.UniqueTags.AppendTags(
						OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags.FilterExact(StartCellInputData->AssemblyTags));
				}
				if (StartCellInputData->AssemblyTags.HasAnyExact(OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags))
				{
					OrganContextPtr->PlacedTagGroups.RequiredAnyTags.AppendTags(
						OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags.FilterExact(StartCellInputData->AssemblyTags));
				}

				// Add tags to context
				if (!StartCellInputData->ContextTagsAdded.IsEmpty())
				{
					OrganContextPtr->ContextTags.AppendTags(StartCellInputData->ContextTagsAdded);
				}

				// Apply this cell's counter operations in author order. Reversed 1:1 in RemoveCellNode,
				// so operations are expected to be Add/Subtract only (Multiply/Divide are not invertible).
				for (int32 j = 0; j < StartCellInputData->TagCounterOperations.Num(); j++)
				{
					OrganContextPtr->TagCounter.ApplyOperation(StartCellInputData->TagCounterOperations[j]);
				}

				// Link our nodes
				BoneNode->Link(StartNode);
				StartNode->LinkJunction(TargetJunctionKey, BoneNode);

				// Base node connection
				BoneNode->Connect(StartNode);

				// Placed successfully - no need to try this cell's remaining junctions.
				break;
			}
		}
	}
	while (OrganContextPtr->CellGraph == nullptr);
}

bool FNOrganGraphBuilderTask::DoesWorldCollide(const FNAssemblyGraphCellNode* CellNode) const
{
	const float WorldHullPenetration = OrganContextPtr->WorldHullPenetration;
	const TArray<FNRawMesh>& WorldCollisionMeshes = WorldContextPtr->WorldCollisionMeshes;
	for (int32 i = 0; i < WorldCollisionMeshes.Num(); i++)
	{
		const float PenetrationDepth = CellNode->GetHullIntersectDepth(WorldCollisionMeshes[i], WorldHullPenetration);
		if (PenetrationDepth == 0.0f)
		{
			if (CellNode->CheckHullIntersects(WorldCollisionMeshes[i]))
			{
				return true;
			}
			continue;
		}
		if (PenetrationDepth >= WorldHullPenetration)
		{
			return true;
		}
	}
	return false;
}

bool FNOrganGraphBuilderTask::DoesExistingNodeWorldCollide(const FNAssemblyGraphCellNode* CellNode) const
{
	const float CellHullPenetration = OrganContextPtr->CellHullPenetration;
	const TArray<FNRawMesh>& ExistingNodeCollisionMeshes = WorldContextPtr->NodeCollisionMeshes;
	for (int32 i = 0; i < ExistingNodeCollisionMeshCount; i++)
	{
		const float PenetrationDepth = CellNode->GetHullIntersectDepth(ExistingNodeCollisionMeshes[i], CellHullPenetration);
		if (PenetrationDepth == 0.0f)
		{
			if (CellNode->CheckHullIntersects(ExistingNodeCollisionMeshes[i]))
			{
				return true;
			}
			continue;
		}
		if (PenetrationDepth >= CellHullPenetration)
		{
			return true;
		}
	}
	return false;
}

TArray<FNAssemblyGraphCellNode*> FNOrganGraphBuilderTask::CheckNodeBounds(const FNAssemblyGraphCellNode* NewNode) const
{
	TArray<FNAssemblyGraphCellNode*> HitNodes;
	for (const auto RegisteredNode : OrganContextPtr->CellGraph->GetNodes())
	{
		if (RegisteredNode->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

		FNAssemblyGraphCellNode* SourceNode = static_cast<FNAssemblyGraphCellNode*>(RegisteredNode);
		if (SourceNode->CheckBoundsIntersects(NewNode))
		{
			HitNodes.Add(SourceNode);
		}
	}
	return HitNodes;
}

TArray<FNAssemblyGraphCellNode*> FNOrganGraphBuilderTask::CheckNodeHull(FNAssemblyGraphCellNode* NewNode) const
{
	TArray<FNAssemblyGraphCellNode*> HitNodes;
	for (const auto RegisteredNode : OrganContextPtr->CellGraph->GetNodes())
	{
		if (RegisteredNode->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

		FNAssemblyGraphCellNode* SourceNode = static_cast<FNAssemblyGraphCellNode*>(RegisteredNode);
		if (SourceNode->CheckHullIntersects(NewNode))
		{
			HitNodes.Add(SourceNode);
		}
	}
	return HitNodes;
}

TArray<FNAssemblyGraphNode*> FNOrganGraphBuilderTask::ProcessNode(FNMersenneTwister& Random, FNAssemblyGraphNode* SourceNode, const bool bIsEndNode) const
{
	switch (SourceNode->GetNodeType())
	{
		using enum ENAssemblyGraphNodeType;
	case Cell:
		return ProcessCellNode(Random, static_cast<FNAssemblyGraphCellNode*>(SourceNode), bIsEndNode);
	case Bone:
	case Null:
	default:
		break;
	}
	return TArray<FNAssemblyGraphNode*>();
}

TArray<FNAssemblyGraphNode*> FNOrganGraphBuilderTask::ProcessCellNode(FNMersenneTwister& Random, FNAssemblyGraphCellNode* SourceCellNode, const bool bIsEndNode) const
{
	TMap<int32, FNCellJunctionDetails*> OpenJunctions = SourceCellNode->GetOpenJunctions();
	FNWeightedIntegerArray WeightedOpenJunctionKeys;
	for (const auto Junction : OpenJunctions)
	{
		// Double the likelihood of using a required junction
		if (Junction.Value->Requirements == ENCellJunctionRequirements::Required)
		{
			WeightedOpenJunctionKeys.Add(Junction.Key, Junction.Value->Weighting * 2);
		}
		else
		{
			WeightedOpenJunctionKeys.Add(Junction.Key, Junction.Value->Weighting);
		}
	}
	const int32 OpenJunctionCount = OpenJunctions.Num();
	
	TArray<FNAssemblyGraphNode*> NewNodes;
	
	FNWeightedIntegerArray CellInputWeightedIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	
	
	for (int32 i = 0; i < OpenJunctionCount; ++i)
	{
		if (OrganContextPtr->MaximumCellCount > 0 &&
			OrganContextPtr->CellGraph->GetCellNodeCount() >= OrganContextPtr->MaximumCellCount)
		{
			break;
		}

		// Select the next junction to fill, using weighted priority.
		int32 SourceJunctionKey = WeightedOpenJunctionKeys.TwistedValueAndRemove(Random);
		FNCellJunctionDetails* SourceJunctionValue = OpenJunctions[SourceJunctionKey];

		// We're going to need the desired target rotation so that when we generate our possible list we account for the rotational allowance
		FQuat SourceJunctionWorldQuat = SourceJunctionValue->WorldRotation.Quaternion();
		
		// Build our possible list of cells (and cache out the valid junctions)
		FNCellInputDataFilter NodeFilter;
		NodeFilter.NodeDepth = SourceCellNode->GetNodeDepth();
		NodeFilter.SocketSize = SourceJunctionValue->SocketSize;
		NodeFilter.SourceQuat = SourceJunctionWorldQuat;
		NodeFilter.SourceCellInputData = SourceCellNode->GetInputDataPtr();
		NodeFilter.SourceCellNode = SourceCellNode;
		// Direction constraints measure the bearing from the organ start to where the candidate will attach. The open
		// junction's world location is that attach point; using the parent cell's centre instead reads the parent's
		// bearing, which lets e.g. an East-constrained cell pass while it actually lands to the west of the bone.
		NodeFilter.WorldPosition = SourceJunctionValue->WorldLocation;
		NodeFilter.bIsEndNode = bIsEndNode;

		OrganContextPtr->FilterCellInputData(NodeFilter, CellInputWeightedIndices, ValidJunctions);
		
		// We don't have any cell input data able OR junctions to fill this spot, so we have to null it out. We will add a NullNode to the graph and connect it up.
		if (CellInputWeightedIndices.WeightedCount() == 0 || ValidJunctions.IsEmpty())
		{
			// TODO: We will later go back and fill this with something.
			FNAssemblyGraphNullNode* NullNode = FNAssemblyGraphNodeFactory::CreateNullNode(SourceJunctionValue->WorldLocation, SourceJunctionValue->WorldRotation);
			
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_AddNullNode)
			
			OrganContextPtr->CellGraph->RegisterNode(NullNode);
			SourceCellNode->LinkJunction(SourceJunctionKey, NullNode);
			NullNode->Link(SourceCellNode);
			SourceCellNode->Connect(NullNode);
			continue;
		}
		
		// Pick our cell to use to spawn
		const int32 CellInputIndex = CellInputWeightedIndices.TwistedValue(Random);
		FNVirtualCellData* CellInputData = &OrganContextPtr->CellInputData[CellInputIndex];
		
		// Pick the junction of the cell we are going to use
		TArray<int32>& ValidJunctionIndices = ValidJunctions[CellInputIndex];
		if (ValidJunctionIndices.IsEmpty())
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Cell input index produced no valid junctions. Adding null node?"));
			
			// TODO: We will later go back and fill this with something.
			FNAssemblyGraphNullNode* NullNode = FNAssemblyGraphNodeFactory::CreateNullNode(SourceJunctionValue->WorldLocation, SourceJunctionValue->WorldRotation);
			
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_AddNullNode)
			
			OrganContextPtr->CellGraph->RegisterNode(NullNode);
			SourceCellNode->LinkJunction(SourceJunctionKey, NullNode);
			NullNode->Link(SourceCellNode);
			SourceCellNode->Connect(NullNode);
			continue;
			
		}
		
		// Resolve geometry, run spatial validation, and link the chosen candidate. Shared with the finisher-minimum
		// guarantee pass so both placement paths apply identical rotation/bounds/collision rules.
		FNAssemblyGraphCellNode* TargetCellNode = TryAttachCellToJunction(
			Random, SourceCellNode, SourceJunctionKey, SourceJunctionValue, SourceJunctionWorldQuat, CellInputData, ValidJunctionIndices);
		if (TargetCellNode != nullptr)
		{
			NewNodes.Add(TargetCellNode);
		}
	}

	return NewNodes;
}

FNAssemblyGraphCellNode* FNOrganGraphBuilderTask::TryAttachCellToJunction(FNMersenneTwister& Random, FNAssemblyGraphCellNode* SourceCellNode,
	const int32 SourceJunctionKey, const FNCellJunctionDetails* SourceJunctionValue, const FQuat& SourceJunctionWorldQuat,
	FNVirtualCellData* CellInputData, const TArray<int32>& ValidJunctionIndices) const
{
	const int32 TargetJunctionKeyIndex = Random.IntegerRange(0, ValidJunctionIndices.Num()-1);
	// Source guarantees this will find something.
	const int32 TargetJunctionKey = ValidJunctionIndices[TargetJunctionKeyIndex];
	const FNCellJunctionDetails* TargetJunctionDetails = CellInputData->Junctions.Find(TargetJunctionKey);

	// Unlike matching to a Bone, when trying to resolve the rotation of a matching one junction to another, we need to find the
	// rotation which makes them face the opposite directions. We flip 180 degrees around the up axis to reverse the forward
	// direction, then inverse the target's local rotation to undo it before applying the world rotation (same pattern as bone-to-junction).
	FQuat TargetJunctionLocalQuat = TargetJunctionDetails->WorldRotation.Quaternion();
	FQuat RequiredRotationQuat = SourceJunctionWorldQuat * FQuat(FVector::UpVector, PI) * TargetJunctionLocalQuat.Inverse();
	FRotator RequiredRotation = RequiredRotationQuat.Rotator();

	FVector TargetJunctionWorldOffset = RequiredRotationQuat.RotateVector(TargetJunctionDetails->WorldLocation);
	FVector TargetJunctionWorldPosition = SourceJunctionValue->WorldLocation - TargetJunctionWorldOffset;

	FNAssemblyGraphNodeParams NodeParams;
	NodeParams.ContextTagsAdded = CellInputData->ContextTagsAdded;
	NodeParams.AssemblyTags = CellInputData->AssemblyTags;
	NodeParams.Seed = Random.UnsignedInteger64();
	NodeParams.WorldPosition = TargetJunctionWorldPosition;
	NodeParams.WorldRotation = RequiredRotation;
	FNAssemblyGraphCellNode* TargetCellNode = FNAssemblyGraphNodeFactory::CreateCellNode(NodeParams, CellInputData, OrganContextPtr->VoxelSize);

	// Reject the node if it falls outside the organ's bounds. Check the AABB first (cheap), then fall back to the
	// tighter hull check. If neither fits inside Context->Bounds we discard and move on, skip the whole thing if the organ was unbounded.
	if (!OrganContextPtr->bUnbound)
	{
		const FBox ContextBoundsBox = OrganContextPtr->Bounds.GetBox();
		if (!TargetCellNode->IsBoundsInside(ContextBoundsBox) && !TargetCellNode->IsHullInside(ContextBoundsBox))
		{
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardOutOfBoundsCellNode)
			delete TargetCellNode;
			return nullptr;
		}
	}

	// Check world collision
	if (DoesWorldCollide(TargetCellNode))
	{
		N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardWorldCollidingCellNode)
		delete TargetCellNode;
		return nullptr;
	}

	// Check previous pass existing node world collision (this doesn't check concurrent pass as they have been already filtered as outside its collision)
	if (DoesExistingNodeWorldCollide(TargetCellNode))
	{
		N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardExistingNodeWorldCollidingCellNode)
		delete TargetCellNode;
		return nullptr;
	}

	// Now check the bounds of other existing nodes
	TArray<FNAssemblyGraphCellNode*> BoundsIntersectingNodes = CheckNodeBounds(TargetCellNode);
	const float CellHullPenetration = OrganContextPtr->CellHullPenetration;
	for (int32 j = BoundsIntersectingNodes.Num() - 1; j >= 0; j--)
	{
		if (BoundsIntersectingNodes[j] == SourceCellNode)
		{
			// Junction-connected pair: tolerate overlap up to CellHullPenetration
			const float PenetrationDepth = SourceCellNode->GetHullIntersectDepth(TargetCellNode, CellHullPenetration);
			if (PenetrationDepth < CellHullPenetration)
			{
				BoundsIntersectingNodes.RemoveAt(j);
			}
		}
		else if (!BoundsIntersectingNodes[j]->CheckHullIntersects(TargetCellNode))
		{
			BoundsIntersectingNodes.RemoveAt(j);
		}
	}

	// Only build when we are not colliding with anything
	if (BoundsIntersectingNodes.IsEmpty())
	{
		// Our cell has unique tags that need to get added to the used
		if (CellInputData->AssemblyTags.HasAnyExact(OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags))
		{
			OrganContextPtr->PlacedTagGroups.UniqueTags.AppendTags(
				OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags.FilterExact(CellInputData->AssemblyTags));
		}
		if (CellInputData->AssemblyTags.HasAnyExact(OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags))
		{
			OrganContextPtr->PlacedTagGroups.RequiredAnyTags.AppendTags(
				OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags.FilterExact(CellInputData->AssemblyTags));
		}

		// Add tags to context
		if (!CellInputData->ContextTagsAdded.IsEmpty())
		{
			OrganContextPtr->ContextTags.AppendTags(CellInputData->ContextTagsAdded);
		}

		// Apply this cell's counter operations in author order. Reversed 1:1 in RemoveCellNode,
		// so operations are expected to be Add/Subtract only (Multiply/Divide are not invertible).
		for (int32 j = 0; j < CellInputData->TagCounterOperations.Num(); j++)
		{
			OrganContextPtr->TagCounter.ApplyOperation(CellInputData->TagCounterOperations[j]);
		}

		// We've passed validation, lets register it and move on
		OrganContextPtr->CellGraph->RegisterNode(TargetCellNode);

		SourceCellNode->LinkJunction(SourceJunctionKey, TargetCellNode);
		TargetCellNode->LinkJunction(TargetJunctionKey, SourceCellNode);

		SourceCellNode->Connect(TargetCellNode);

		N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_AddCellNode)
		return TargetCellNode;
	}

	N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardIntersectingCellNode)
	delete TargetCellNode;
	return nullptr;
}

void FNOrganGraphBuilderTask::CapBranchesWithFinishers(FNMersenneTwister& Random) const
{
	if (!OrganContextPtr->CellInputDataSummary.bFoundFinisherTagged &&
		!OrganContextPtr->CellInputDataSummary.bFoundFinisherOnlyTagged)
	{
		return;
	}

	TArray<FNAssemblyGraphNode*> OpenNodes = OrganContextPtr->CellGraph->GetNodesWithOpenJunctions();
#if !UE_BUILD_SHIPPING
	int CapNum = 0;
#endif // !UE_BUILD_SHIPPING
	for (int32 j = 0; j < OpenNodes.Num(); j++)
	{
#if !UE_BUILD_SHIPPING		
		CapNum += ProcessNode(Random, OpenNodes[j], true).Num();
#else
		ProcessNode(Random, OpenNodes[j], true);
#endif // !UE_BUILD_SHIPPING		
	}
#if !UE_BUILD_SHIPPING
	N_ASSEMBLY_ANALYTICS_TWO_PARAM(OrganGraphBuilder_CappedWithFinisher, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, CapNum);
#endif // !UE_BUILD_SHIPPING
	
}

void FNOrganGraphBuilderTask::EnsureFinisherMinimums(FNMersenneTwister& Random) const
{
	// Cells governed by a combined Unique + RequiredAny group are validated by the RequiredAny check rather than
	// their per-cell minimum (mirrors CheckGraph), so they are not candidates for forcing.
	const FGameplayTagContainer UniqueAndRequiredTags =
		OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags.FilterExact(OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags);

	// Gather targets first — no RNG, no graph mutation. When nothing needs forcing this whole pass is a true no-op,
	// so tissues that never pair a finisher with a MinimumCount keep byte-identical generation.
	TArray<int32> TargetCellIndices;
	for (int32 i = 0; i < OrganContextPtr->CellInputData.Num(); i++)
	{
		if (FNVirtualOrganContext::IsUnmetFinisherMinimum(OrganContextPtr->CellInputData[i], UniqueAndRequiredTags))
		{
			TargetCellIndices.Add(i);
		}
	}
	if (TargetCellIndices.IsEmpty())
	{
		return;
	}

	for (const int32 TargetCellIndex : TargetCellIndices)
	{
		const FNVirtualCellData& TargetCell = OrganContextPtr->CellInputData[TargetCellIndex];
		while (TargetCell.UsedCount < TargetCell.MinimumCount)
		{
			// Respect cooperative cancellation and the global cell ceiling, matching the rest of the build.
			if (TaskGraphContextPtr->IsCancelled()) return;
			if (OrganContextPtr->MaximumCellCount > 0 &&
				OrganContextPtr->CellGraph->GetCellNodeCount() >= OrganContextPtr->MaximumCellCount)
			{
				break;
			}

			// No open junction can currently host this cell. CheckGraph will then reject the graph and the build
			// retries from a fresh attempt — the correct outcome for a layout that genuinely cannot place it.
			if (!TryPlaceTargetCellOnce(Random, TargetCellIndex))
			{
				break;
			}
		}
	}
}

bool FNOrganGraphBuilderTask::TryPlaceTargetCellOnce(FNMersenneTwister& Random, const int32 TargetCellIndex) const
{
	const TArray<FNAssemblyGraphNode*> OpenNodes = OrganContextPtr->CellGraph->GetNodesWithOpenJunctions();
	for (FNAssemblyGraphNode* OpenNode : OpenNodes)
	{
		if (OpenNode->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;
		FNAssemblyGraphCellNode* SourceCellNode = static_cast<FNAssemblyGraphCellNode*>(OpenNode);

		for (const TPair<int32, FNCellJunctionDetails*>& Junction : SourceCellNode->GetOpenJunctions())
		{
			const int32 SourceJunctionKey = Junction.Key;
			FNCellJunctionDetails* SourceJunctionValue = Junction.Value;

			// End-node filter so Finisher/FinisherOnly cells are eligible; reuses every geometry/tag/Max/Unique gate.
			FNCellInputDataFilter NodeFilter;
			NodeFilter.NodeDepth = SourceCellNode->GetNodeDepth();
			NodeFilter.SocketSize = SourceJunctionValue->SocketSize;
			NodeFilter.SourceQuat = SourceJunctionValue->WorldRotation.Quaternion();
			NodeFilter.SourceCellInputData = SourceCellNode->GetInputDataPtr();
			NodeFilter.SourceCellNode = SourceCellNode;
			NodeFilter.WorldPosition = SourceJunctionValue->WorldLocation;
			NodeFilter.bIsEndNode = true;

			FNWeightedIntegerArray CellInputWeightedIndices;
			TMap<int32, TArray<int32>> ValidJunctions;
			OrganContextPtr->FilterCellInputData(NodeFilter, CellInputWeightedIndices, ValidJunctions);

			// Only proceed when the target cell itself is a valid candidate for this specific junction. Filtering
			// consumes no RNG, so probing junctions that cannot host the cell leaves the stream untouched.
			const TArray<int32>* ValidJunctionIndices = ValidJunctions.Find(TargetCellIndex);
			if (ValidJunctionIndices == nullptr || ValidJunctionIndices->IsEmpty())
			{
				continue;
			}

			const FQuat SourceJunctionWorldQuat = SourceJunctionValue->WorldRotation.Quaternion();
			FNVirtualCellData* TargetCellInputData = &OrganContextPtr->CellInputData[TargetCellIndex];
			if (TryAttachCellToJunction(Random, SourceCellNode, SourceJunctionKey, SourceJunctionValue,
				SourceJunctionWorldQuat, TargetCellInputData, *ValidJunctionIndices) != nullptr)
			{
				return true;
			}
		}
	}
	return false;
}

void FNOrganGraphBuilderTask::RemoveCellNode(FNAssemblyGraphCellNode* CellNode) const
{
	const TMap<int32, FNCellJunctionDetails>& Junctions = CellNode->GetJunctions();
	TArray<int32> JunctionKeys;
	Junctions.GetKeys(JunctionKeys);

	for (const int32 JunctionKey : JunctionKeys)
	{
		FNAssemblyGraphNode* LinkedNode = CellNode->GetLinkedNode(JunctionKey);
		if (LinkedNode == nullptr) continue;

		if (LinkedNode->GetNodeType() == ENAssemblyGraphNodeType::Cell)
		{
			FNAssemblyGraphCellNode* LinkedCell = static_cast<FNAssemblyGraphCellNode*>(LinkedNode);
			const int32 ReverseKey = LinkedCell->FindJunctionKeyLinkedTo(CellNode);
			if (ReverseKey != INDEX_NONE)
			{
				LinkedCell->UnlinkJunction(ReverseKey);
			}
			CellNode->UnlinkJunction(JunctionKey);
			CellNode->Disconnect(LinkedNode);
		}
		else if (LinkedNode->GetNodeType() == ENAssemblyGraphNodeType::Null)
		{
			FNAssemblyGraphNullNode* NullNode = static_cast<FNAssemblyGraphNullNode*>(LinkedNode);
			NullNode->Unlink();
			CellNode->UnlinkJunction(JunctionKey);
			CellNode->Disconnect(NullNode);
			OrganContextPtr->CellGraph->UnregisterNode(NullNode);
			delete NullNode;
		}
	}
	
	// Reverse unique-tag tracking so the cell template becomes eligible again
	FNVirtualCellData* InputData = CellNode->GetInputDataPtr();
	if (InputData != nullptr && InputData->AssemblyTags.HasAnyExact(OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags))
	{
		OrganContextPtr->PlacedTagGroups.UniqueTags.RemoveTags(
			OrganContextPtr->CellInputDataSummary.GroupTags.UniqueTags.FilterExact(InputData->AssemblyTags));
	}

	// We need to handle the must-have tags and ContextTags differently, figure out what is not covered when this is removed.
	const bool bTrackRequired = InputData != nullptr
		&& InputData->AssemblyTags.HasAnyExact(OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags);

	FGameplayTagContainer UncoveredRequiredAnyTags;
	if (bTrackRequired)
	{
		UncoveredRequiredAnyTags = InputData->AssemblyTags.FilterExact(OrganContextPtr->CellInputDataSummary.GroupTags.RequiredAnyTags);
	}

	// Use cached copy
	FGameplayTagContainer UncoveredContextTags = CellNode->GetContextTagsAdded();
	UncoveredContextTags.RemoveTags(OrganContextPtr->BaseContextTags);

	if (bTrackRequired || !UncoveredContextTags.IsEmpty())
	{
		const TArray<FNAssemblyGraphNode*>& Nodes = OrganContextPtr->CellGraph->GetNodes();
		for (const FNAssemblyGraphNode* Node : Nodes)
		{
			// Skip same cell, and non cell types
			if (Node == CellNode || Node->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

			const FNVirtualCellData* OtherData = static_cast<const FNAssemblyGraphCellNode*>(Node)->GetInputDataPtr();
			if (OtherData == nullptr) continue;

			if (bTrackRequired) UncoveredRequiredAnyTags.RemoveTags(OtherData->AssemblyTags);
			if (!UncoveredContextTags.IsEmpty()) UncoveredContextTags.RemoveTags(OtherData->ContextTagsAdded);

			if ((!bTrackRequired || UncoveredRequiredAnyTags.IsEmpty())
				&& UncoveredContextTags.IsEmpty()) break;
		}

		if (bTrackRequired && !UncoveredRequiredAnyTags.IsEmpty())
		{
			OrganContextPtr->PlacedTagGroups.RequiredAnyTags.RemoveTags(UncoveredRequiredAnyTags);
		}

		if (!UncoveredContextTags.IsEmpty())
		{
			OrganContextPtr->ContextTags.RemoveTags(UncoveredContextTags);
		}
	}
	
	// Undo this cell's counter operations in reverse author order, mirroring the apply in ProcessCellNode.
	// Valid only because operations are restricted to Add/Subtract, which are exact inverses; per-cell
	// reversal of Multiply/Divide would not restore the prior value.
	if (InputData != nullptr)
	{
		for (int32 j = InputData->TagCounterOperations.Num() - 1; j >= 0; --j)
		{
			OrganContextPtr->TagCounter.ReverseOperation(InputData->TagCounterOperations[j]);
		}
	}

	OrganContextPtr->CellGraph->UnregisterNode(CellNode);
	delete CellNode;
}

void FNOrganGraphBuilderTask::EnforceNotFinisherConstraint() const
{
	bool bRemovedAny = true;
	while (bRemovedAny)
	{
		bRemovedAny = false;
		const TArray<FNAssemblyGraphNode*>& Nodes = OrganContextPtr->CellGraph->GetNodes();

		for (int32 i = Nodes.Num() - 1; i >= 0; i--)
		{
			if (Nodes[i]->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Nodes[i]);
			if (!CellNode->GetAssemblyTags().HasTag(NWorldAssembly_Behavior_NotFinisher)) continue;

			// Check if this is a leaf (no Cell-type downstream children)
			bool bHasCellChild = false;
			for (const FNAssemblyGraphNode* Downstream : CellNode->GetDownstreamNodes())
			{
				if (Downstream->GetNodeType() == ENAssemblyGraphNodeType::Cell)
				{
					bHasCellChild = true;
					break;
				}
			}
			if (bHasCellChild) continue;
			N_ASSEMBLY_ANALYTICS_INDEX(OrganGraphBuilder_DiscardDueToNonFinisherConstraint)
			RemoveCellNode(CellNode);
			bRemovedAny = true;
			break; // Restart scan — Nodes array was modified
		}
	}
}
