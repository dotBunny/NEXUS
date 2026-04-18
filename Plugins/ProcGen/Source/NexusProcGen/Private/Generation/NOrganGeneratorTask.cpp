// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

#include "NProcGenMinimal.h"
#include "Generation/NProcGenGraphBoneNode.h"
#include "Generation/NProcGenGraphCellNode.h"
#include "Generation/NProcGenGraphNodeFactory.h"
#include "Math/NMersenneTwister.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
	: Context(ContextPtr.ToSharedRef()), PassContext(PassContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{
}

// TODO: Spin up multiple of these tasks? with different sub-seeds? Maybe we make a seperate task under this

// REMINDERS
// - Need a quick way to make sure that a spawned cell is INSIDE the volume


void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// The context was not validated during creation, so we cannot process it
	if (!Context->IsValid()) return;
	
	// Create our deterministic random for the task which will get passed byref to sub-methods
	FNMersenneTwister Random(Context->Seed);
	
	// Find the bone and build our starting cell
	StartGraph(Random);
	
	// Check for start placement
	int NodeCount = Context->CellGraph->GetNodeCount();
	if (NodeCount == 0) { return; }
	
	// TEMP TEST: Process our starting node
	// --- We could make a thing that randomly picks an open node? to process next?
	
	// TEMP Iteration count generation
	TArray<FNProcGenGraphNode*> NewNodes = ProcessNode(Random, Context->CellGraph->GetLastNode());
	for (int i = 0; i < 5; i++)
	{
		TArray<FNProcGenGraphNode*> NextBatch = NewNodes;
		NewNodes.Empty();
		for (int j = 0; j < NextBatch.Num(); j++)
		{
			TArray<FNProcGenGraphNode*> BatchNodes = ProcessNode(Random, NextBatch[j]);
			for (int k = 0; k < BatchNodes.Num(); k++)
			{
				NewNodes.Add(BatchNodes[k]);
			}
		}
	}
	
	if (Context->CellGraph->GetNodeCount() < 10)
	{
		TArray<FNProcGenGraphNode*> OpenNodes = Context->CellGraph->GetNodesWithOpenJunctions();
		for (int j = 0; j < OpenNodes.Num(); j++)
		{
			ProcessNode(Random, OpenNodes[j]);
		}
	}

	Context->bSuccessful = true;
}

void FNOrganGeneratorTask::StartGraph(FNMersenneTwister& Random) const
{
	// TODO: We haven't resolved yet how we might join multiple generation points yet so we are just going to use the first bone.
	FNBoneInputData& BoneData = Context->BoneInputData[0];
	
	FNCellInputDataFilter PreFilter;
	PreFilter.SocketSize = BoneData.SocketSize;
	PreFilter.SourceQuat = FQuat(BoneData.WorldRotation);
	PreFilter.bRequireStart = true;
	
	FNWeightedIntegerArray WeightedStartIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	Context->FilterCellInputData(PreFilter, WeightedStartIndices, ValidJunctions);
	
	// Unable to generate
	if (WeightedStartIndices.Count() == 0)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("Unable to place starting cell, due to no valid cells available."))
		return;
	}
	
	// Get a weighted array of indices representing possible starting cells, and reference the cell data
	const int32 StartCellIndex = WeightedStartIndices.TwistedValue(Random);
	FNCellInputData StartCellInputData = Context->CellInputData[StartCellIndex];
	
	// Decide which appropriately sized junction is going to be used
	TArray<int32>& ValidJunctionIndices = ValidJunctions[StartCellIndex];
	const int TargetJunctionKeyIndex = Random.IntegerRange(0, ValidJunctionIndices.Num()-1);
	const int TargetJunctionKey = ValidJunctionIndices[TargetJunctionKeyIndex];
	
	const FNCellJunctionDetails* StartCellJunctionDetails = StartCellInputData.Junctions.Find(TargetJunctionKey);
	
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
	FNProcGenGraphBoneNode* BoneNode = FNProcGenGraphNodeFactory::CreateBoneNode(&BoneData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph = MakeUnique<FNProcGenGraph>(BoneNode);
	
	// Create our first cell node, attaching it to the bone node
	FNProcGenGraphCellNode* StartNode = FNProcGenGraphNodeFactory::CreateCellNode(&StartCellInputData, CellWorldPosition, CellWorldRotation);
	Context->CellGraph->RegisterNode(StartNode);
	
	// Link our nodes
	BoneNode->Link(StartNode);
	StartNode->Link(TargetJunctionKey, BoneNode);
}

TArray<FNProcGenGraphCellNode*> FNOrganGeneratorTask::CheckNodeBounds(FNProcGenGraphCellNode* NewNode) const
{
	TArray<FNProcGenGraphCellNode*> HitNodes;
	for (const auto RegisteredNode : Context->CellGraph->GetNodes())
	{
		if (RegisteredNode->GetNodeType() != ENProcGenGraphNodeType::Cell) continue;

		FNProcGenGraphCellNode* SourceNode = static_cast<FNProcGenGraphCellNode*>(RegisteredNode);
		if (SourceNode->CheckBoundsIntersects(NewNode))
		{
			HitNodes.Add(SourceNode);
		}
	}
	return MoveTemp(HitNodes);
}

TArray<FNProcGenGraphCellNode*> FNOrganGeneratorTask::CheckNodeHull(FNProcGenGraphCellNode* NewNode) const
{
	TArray<FNProcGenGraphCellNode*> HitNodes;
	for (const auto RegisteredNode : Context->CellGraph->GetNodes())
	{
		if (RegisteredNode->GetNodeType() != ENProcGenGraphNodeType::Cell) continue;

		FNProcGenGraphCellNode* SourceNode = static_cast<FNProcGenGraphCellNode*>(RegisteredNode);
		if (SourceNode->CheckHullIntersects(NewNode))
		{
			HitNodes.Add(SourceNode);
		}
	}
	return MoveTemp(HitNodes);
}

TArray<FNProcGenGraphNode*> FNOrganGeneratorTask::ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const
{
	switch (SourceNode->GetNodeType())
	{
		using enum ENProcGenGraphNodeType;
	case Cell:
		return ProcessCellNode(Random, static_cast<FNProcGenGraphCellNode*>(SourceNode));
	case Bone:
	case Null:
	default:
		break;
	}
	return TArray<FNProcGenGraphNode*>();
}

TArray<FNProcGenGraphNode*> FNOrganGeneratorTask::ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const
{
	TMap<int32, FNCellJunctionDetails*> OpenJunctions = SourceCellNode->GetOpenJunctions();
	TArray<FNProcGenGraphNode*> NewNodes;
	
	FNWeightedIntegerArray CellInputWeightedIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	
	for (const auto Junction : OpenJunctions)
	{
		// We're going to need the desired target rotation so that when we generate our possible list we account for the rotational allowance
		FQuat SourceJunctionWorldQuat = Junction.Value->WorldRotation.Quaternion();
		
		// Build our possible list of cells (and cache out the valid junctions)
		FNCellInputDataFilter NodeFilter;
		NodeFilter.SocketSize = Junction.Value->SocketSize;
		NodeFilter.SourceQuat = SourceJunctionWorldQuat;
		Context->FilterCellInputData(NodeFilter, CellInputWeightedIndices, ValidJunctions);
		
		// We don't have any cell input data able to fill this spot, so we have to null it out. We will add a NullNode to the graph and connect it up.
		if (CellInputWeightedIndices.Count() == 0)
		{
			// TODO: We will later go back and fill this with something.
			FNProcGenGraphNullNode* NullNode = FNProcGenGraphNodeFactory::CreateNullNode(Junction.Value->WorldLocation, Junction.Value->WorldRotation);
			Context->CellGraph->RegisterNode(NullNode);
			
			SourceCellNode->Link(Junction.Key, NullNode);
			NullNode->Link(SourceCellNode);
			continue;
		}
		
		// Pick our cell to use to spawn
		const int32 CellInputIndex = CellInputWeightedIndices.TwistedValue(Random);
		FNCellInputData CellInputData = Context->CellInputData[CellInputIndex];
		
		// Pick the junction of the cell we are going to use
		TArray<int32>& ValidJunctionIndices = ValidJunctions[CellInputIndex];
		const int TargetJunctionKeyIndex = Random.IntegerRange(0, ValidJunctionIndices.Num()-1);
		const int TargetJunctionKey = ValidJunctionIndices[TargetJunctionKeyIndex];
		const FNCellJunctionDetails* TargetJunctionDetails = CellInputData.Junctions.Find(TargetJunctionKey);
		
		// Unlike matching to a Bone, when trying to resolve the rotation of a matching one junction to another, we need to find the
		// rotation which makes them face the opposite directions. We flip 180 degrees around the up axis to reverse the forward
		// direction, then inverse the target's local rotation to undo it before applying the world rotation (same pattern as bone-to-junction).
		FQuat TargetJunctionLocalQuat = TargetJunctionDetails->WorldRotation.Quaternion();
		FQuat RequiredRotationQuat = SourceJunctionWorldQuat * FQuat(FVector::UpVector, PI) * TargetJunctionLocalQuat.Inverse();
		FRotator RequiredRotation = RequiredRotationQuat.Rotator(); 
		
		FVector TargetJunctionWorldOffset = RequiredRotationQuat.RotateVector(TargetJunctionDetails->WorldLocation);
		FVector TargetJunctionWorldPosition = Junction.Value->WorldLocation - TargetJunctionWorldOffset;
	
		FNProcGenGraphCellNode* TargetCellNode = FNProcGenGraphNodeFactory::CreateCellNode(&CellInputData, TargetJunctionWorldPosition, RequiredRotation);
		
		// Reject the node if it falls outside the organ's bounds. Check the AABB first (cheap), then fall back to the
		// tighter hull check. If neither fits inside Context->Bounds we discard and move on, skip the whole thing if the organ was unbounded.
		if (!Context->bUnbounded)
		{
			const FBox ContextBoundsBox = Context->Bounds.GetBox();
			if (!TargetCellNode->IsBoundsInside(ContextBoundsBox) && !TargetCellNode->IsHullInside(ContextBoundsBox))
			{
				delete TargetCellNode;
				continue;
			}
		}
		
		// Now check the bounds of other existing nodes
		TArray<FNProcGenGraphCellNode*> BoundsIntersectingNodes = CheckNodeBounds(TargetCellNode);
		for (int i = BoundsIntersectingNodes.Num() - 1; i >= 0; i--)
		{
			// Refine the bounds check to look to see if the node violates the hull as it is a tighter collision check.
			if (!BoundsIntersectingNodes[i]->CheckHullIntersects(TargetCellNode))
			{
				BoundsIntersectingNodes.RemoveAt(i);
			}
		}
		
		// Only build when we are not colliding with anything
		if (BoundsIntersectingNodes.Num() == 0)
		{
			// We've passed validation, lets register it and move on
			Context->CellGraph->RegisterNode(TargetCellNode);
			SourceCellNode->Link(Junction.Key, TargetCellNode);
			TargetCellNode->Link(TargetJunctionKey, SourceCellNode);
			NewNodes.Add(TargetCellNode);
		}
		else
		{
			// TODO: this is gonna throw in SonarQube
			delete TargetCellNode;
		}
	}
	
	return MoveTemp(NewNodes);
}
