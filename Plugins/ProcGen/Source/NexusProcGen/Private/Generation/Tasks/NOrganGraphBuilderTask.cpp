// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NOrganGraphBuilderTask.h"

#include "NProcGenMinimal.h"
#include "Generation/Graph/NProcGenGraphBoneNode.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Generation/Graph/NProcGenGraphNodeFactory.h"
#include "Generation/Contexts/NWorldContext.h"
#include "Math/NMersenneTwister.h"

FNOrganGraphBuilderTask::FNOrganGraphBuilderTask(const TSharedPtr<FNOrganContext>& ContextPtr,
	const TSharedPtr<FNGraphCollectionContext>& PassContextPtr, const TSharedPtr<FNWorldContext>& WorldContextPtr)
	: ContextPtr(ContextPtr.ToSharedRef()), PassContextPtr(PassContextPtr.ToSharedRef()), WorldContextPtr(WorldContextPtr.ToSharedRef())
{
}

void FNOrganGraphBuilderTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// The context was not validated during creation, so we cannot process it
	if (!ContextPtr->IsValid()) return;
	
	// Create our deterministic random for the task which will get passed byref to sub-methods
	FNMersenneTwister Random(ContextPtr->GetSeed());
	
	// We're going to copy the state of world collision at the start here, we know they're filled cause of dep chain.
	WorldCollisionMeshes = WorldContextPtr->WorldCollisionMeshes;
	WorldCollisionLocations = WorldContextPtr->WorldCollisionMeshLocations;
	WorldCollisionRotations = WorldContextPtr->WorldCollisionMeshRotations;
	
	// TODO: If this is an unbounded volume should we be adding in all the previous creations to the context (yes)
	
	while (!ContextPtr->IsSuccessful())
	{
		// Find the bone and build our starting cell
		StartGraph(Random);
	
		// Check for start placement and that it was a node too
		if (ContextPtr->CellGraph == nullptr) return;
		int NodeCount = ContextPtr->CellGraph->GetNodeCount();
		if (NodeCount == 0) { return; }
	
		// Initialize Analytics
		Analytics.Init(ContextPtr->GetName(), ContextPtr->MinimumCellCount, ContextPtr->MaximumCellCount, ContextPtr->MaximumRetryCount);
		
		// TODO: Create better logic for placement / etc.
		// #START Temp Generation -------------------------------------------------------------------------------------------------------------------
		TArray<FNProcGenGraphNode*> NewNodes = ProcessNode(Random, ContextPtr->CellGraph->GetLastNode());
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
	
		if (ContextPtr->CellGraph->GetNodeCount() < 10)
		{
			TArray<FNProcGenGraphNode*> OpenNodes = ContextPtr->CellGraph->GetNodesWithOpenJunctions();
			for (int j = 0; j < OpenNodes.Num(); j++)
			{
				ProcessNode(Random, OpenNodes[j]);
			}
		}
		// #END Temp Generation ---------------------------------------------------------------------------------------------------------------------
		
		// At this point we need to validate the graph against the overall requirements from the input settings.
		// The bSuccessful flag gets set at this point, but if it isn't valid we then need to take our chances and regenerate,
		// but only if we have not run out of attempts.
		if (!ContextPtr->ValidateGraph() && !ContextPtr->ResetForRetry(Analytics))
		{
			break;
		}
	}
	
	Analytics.OutputToLog();
	
	// Clean up graph of pointers that can't be kept
	if (ContextPtr->CellGraph != nullptr)
	{
		ContextPtr->CellGraph->CleanupBuilderReferences();
	}
	
	// Only hand off graph if it's good
	if (ContextPtr->IsSuccessful())
	{
		PassContextPtr->TakeGraph(MoveTemp(ContextPtr->CellGraph));
	}
}

void FNOrganGraphBuilderTask::StartGraph(FNMersenneTwister& Random) const
{
	// TODO: We haven't resolved yet how we might join multiple generation points yet so we are just going to use the first bone.
	FNBoneInputData& BoneData = ContextPtr->BoneInputData[0];
	
	FNCellInputDataFilter PreFilter;
	PreFilter.SocketSize = BoneData.SocketSize;
	PreFilter.SourceQuat = FQuat(BoneData.WorldRotation);
	PreFilter.bRequireStart = true;

	FNWeightedIntegerArray WeightedStartIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	ContextPtr->FilterCellInputData(PreFilter, WeightedStartIndices, ValidJunctions);
	
	// Unable to generate
	if (WeightedStartIndices.WeightedCount() == 0)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("Unable to place starting cell, due to no valid cells available."))
		return;
	}
	
	
	// Our starting placement has to happen
	do
	{
		// Get a weighted array of indices representing possible starting cells, and reference the cell data
		const int32 StartCellIndex = WeightedStartIndices.TwistedValue(Random);
		FNCellInputData* StartCellInputData = &ContextPtr->CellInputData[StartCellIndex];
		
		// Decide which appropriately sized junction is going to be used
		TArray<int32>& ValidJunctionIndices = ValidJunctions[StartCellIndex];
		const int TargetJunctionKeyIndex = Random.IntegerRange(0, ValidJunctionIndices.Num()-1);
		const int TargetJunctionKey = ValidJunctionIndices[TargetJunctionKeyIndex];
		
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
		FNProcGenGraphBoneNode* BoneNode = FNProcGenGraphNodeFactory::CreateBoneNode(&BoneData, CellWorldPosition, CellWorldRotation);
		
		// Create our graph
		ContextPtr->CellGraph = MakeUnique<FNProcGenGraph>(
			BoneNode, ContextPtr->Origin, ContextPtr->Bounds, ContextPtr->bUnbounded);
		
		
		// Create our first cell node, attaching it to the bone node
		FNProcGenGraphCellNode* StartNode = FNProcGenGraphNodeFactory::CreateCellNode(StartCellInputData, CellWorldPosition, CellWorldRotation);
		
		if (DoesWorldCollide(StartNode))
		{
			
			delete StartNode; 
			ContextPtr->CellGraph.Reset(); // Bone is already part of graph
			ContextPtr->CellGraph = nullptr;
			Analytics.DiscardStart();
		}
		else
		{
			ContextPtr->CellGraph->RegisterNode(StartNode);

			// Link our nodes
			BoneNode->Link(StartNode);
			StartNode->Link(TargetJunctionKey, BoneNode);
		}
		
		if (Analytics.GetDiscardStart() > 100)
		{
			break;
		}
	} 
	while (ContextPtr->CellGraph == nullptr);
}

bool FNOrganGraphBuilderTask::DoesWorldCollide(const FNProcGenGraphCellNode* CellNode) const
{
	for (int i = 0; i < WorldCollisionMeshes.Num(); i++)
	{
		if (CellNode->CheckHullIntersects(WorldCollisionLocations[i], WorldCollisionRotations[i], WorldCollisionMeshes[i]))
		{
			return true;
		}
	}
	return false;
}

TArray<FNProcGenGraphCellNode*> FNOrganGraphBuilderTask::CheckNodeBounds(FNProcGenGraphCellNode* NewNode) const
{
	TArray<FNProcGenGraphCellNode*> HitNodes;
	for (const auto RegisteredNode : ContextPtr->CellGraph->GetNodes())
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

TArray<FNProcGenGraphCellNode*> FNOrganGraphBuilderTask::CheckNodeHull(FNProcGenGraphCellNode* NewNode) const
{
	TArray<FNProcGenGraphCellNode*> HitNodes;
	for (const auto RegisteredNode : ContextPtr->CellGraph->GetNodes())
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

TArray<FNProcGenGraphNode*> FNOrganGraphBuilderTask::ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const
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

TArray<FNProcGenGraphNode*> FNOrganGraphBuilderTask::ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const
{
	TMap<int32, FNCellJunctionDetails*> OpenJunctions = SourceCellNode->GetOpenJunctions();
	FNWeightedIntegerArray WeightedOpenJunctionKeys;
	for (const auto Junction : OpenJunctions)
	{
		WeightedOpenJunctionKeys.Add(Junction.Key, Junction.Value->Weighting);
	}
	const int32 OpenJunctionCount = OpenJunctions.Num();
	
	TArray<FNProcGenGraphNode*> NewNodes;
	
	FNWeightedIntegerArray CellInputWeightedIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	
	
	for (int i = 0; i < OpenJunctionCount; ++i)
	{
		// Select the next junction to fill, using weighted priority.
		int SourceJunctionKey = WeightedOpenJunctionKeys.TwistedValueAndRemove(Random);
		FNCellJunctionDetails* SourceJunctionValue = OpenJunctions[SourceJunctionKey];

		// We're going to need the desired target rotation so that when we generate our possible list we account for the rotational allowance
		FQuat SourceJunctionWorldQuat = SourceJunctionValue->WorldRotation.Quaternion();
		
		// Build our possible list of cells (and cache out the valid junctions)
		FNCellInputDataFilter NodeFilter;
		
		NodeFilter.SocketSize = SourceJunctionValue->SocketSize;
		NodeFilter.SourceQuat = SourceJunctionWorldQuat;
		NodeFilter.SourceCellInputData = SourceCellNode->GetInputDataPtr(); 
		NodeFilter.SourceCellNode = SourceCellNode;
		
		ContextPtr->FilterCellInputData(NodeFilter, CellInputWeightedIndices, ValidJunctions);
		
		// We don't have any cell input data able to fill this spot, so we have to null it out. We will add a NullNode to the graph and connect it up.
		if (CellInputWeightedIndices.WeightedCount() == 0)
		{
			// TODO: We will later go back and fill this with something.
			FNProcGenGraphNullNode* NullNode = FNProcGenGraphNodeFactory::CreateNullNode(SourceJunctionValue->WorldLocation, SourceJunctionValue->WorldRotation);
			Analytics.AddedNullNode();
			ContextPtr->CellGraph->RegisterNode(NullNode);
			
			SourceCellNode->Link(SourceJunctionKey, NullNode);
			NullNode->Link(SourceCellNode);
			continue;
		}
		
		// Pick our cell to use to spawn
		const int32 CellInputIndex = CellInputWeightedIndices.TwistedValue(Random);
		FNCellInputData* CellInputData = &ContextPtr->CellInputData[CellInputIndex];
		
		// Pick the junction of the cell we are going to use
		TArray<int32>& ValidJunctionIndices = ValidJunctions[CellInputIndex];
		const int TargetJunctionKeyIndex = Random.IntegerRange(0, ValidJunctionIndices.Num()-1);
		const int TargetJunctionKey = ValidJunctionIndices[TargetJunctionKeyIndex];
		const FNCellJunctionDetails* TargetJunctionDetails = CellInputData->Junctions.Find(TargetJunctionKey);
		
		// Unlike matching to a Bone, when trying to resolve the rotation of a matching one junction to another, we need to find the
		// rotation which makes them face the opposite directions. We flip 180 degrees around the up axis to reverse the forward
		// direction, then inverse the target's local rotation to undo it before applying the world rotation (same pattern as bone-to-junction).
		FQuat TargetJunctionLocalQuat = TargetJunctionDetails->WorldRotation.Quaternion();
		FQuat RequiredRotationQuat = SourceJunctionWorldQuat * FQuat(FVector::UpVector, PI) * TargetJunctionLocalQuat.Inverse();
		FRotator RequiredRotation = RequiredRotationQuat.Rotator(); 
		
		FVector TargetJunctionWorldOffset = RequiredRotationQuat.RotateVector(TargetJunctionDetails->WorldLocation);
		FVector TargetJunctionWorldPosition = SourceJunctionValue->WorldLocation - TargetJunctionWorldOffset;
	
		FNProcGenGraphCellNode* TargetCellNode = FNProcGenGraphNodeFactory::CreateCellNode(CellInputData, TargetJunctionWorldPosition, RequiredRotation);
		
		// Reject the node if it falls outside the organ's bounds. Check the AABB first (cheap), then fall back to the
		// tighter hull check. If neither fits inside Context->Bounds we discard and move on, skip the whole thing if the organ was unbounded.
		if (!ContextPtr->bUnbounded)
		{
			const FBox ContextBoundsBox = ContextPtr->Bounds.GetBox();
			if (!TargetCellNode->IsBoundsInside(ContextBoundsBox) && !TargetCellNode->IsHullInside(ContextBoundsBox))
			{
				Analytics.DiscardOutOfBounds();
				delete TargetCellNode;
				continue;
			}
		}
		
		// Check world collision
		if (DoesWorldCollide(TargetCellNode))
		{
			Analytics.DiscardWorldCollision();
			delete TargetCellNode;
			continue;
		}
		
		// Now check the bounds of other existing nodes
		TArray<FNProcGenGraphCellNode*> BoundsIntersectingNodes = CheckNodeBounds(TargetCellNode);
		for (int j = BoundsIntersectingNodes.Num() - 1; j >= 0; j--)
		{
			// Refine the bounds check to look to see if the node violates the hull as it is a tighter collision check.
			if (!BoundsIntersectingNodes[j]->CheckHullIntersects(TargetCellNode))
			{
				BoundsIntersectingNodes.RemoveAt(j);
			}
		}
		
		// Only build when we are not colliding with anything
		if (BoundsIntersectingNodes.Num() == 0)
		{
			// We've passed validation, lets register it and move on
			ContextPtr->CellGraph->RegisterNode(TargetCellNode);
			SourceCellNode->Link(SourceJunctionKey, TargetCellNode);
			TargetCellNode->Link(TargetJunctionKey, SourceCellNode);
			Analytics.AddedCellNode();
			NewNodes.Add(TargetCellNode);
			
			// TODO: Its at this point we could iterate through the graph and see if theres any open junctions near any of this 
			// cell's open junctions and auto link them too. Maybe we do the same task at the end of the graph as well ? 
			// Based on a settings for distance?
		}
		else
		{
			Analytics.DiscardIntersecting();
			delete TargetCellNode;
		}
	}
	
	return MoveTemp(NewNodes);
}
