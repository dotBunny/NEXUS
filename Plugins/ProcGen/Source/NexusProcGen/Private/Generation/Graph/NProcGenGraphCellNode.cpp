// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Graph/NProcGenGraphCellNode.h"

#include "NProcGenMinimal.h"
#include "Math/NVectorUtils.h"

FNProcGenGraphCellNode::FNProcGenGraphCellNode(FNVirtualCellData* InputData, const FVector& Position, const FRotator& Rotation) : FNProcGenGraphNode(Position, Rotation)
{
	// Copy InputData to disconnect from reference
	InputDataPtr = InputData;
	bAlwaysRelevant = InputData->bAlwaysRelevant;
	TemplatePtr = InputDataPtr->Template; // Might not need in future
	FreeJunctionKeys = InputDataPtr->GetJunctionKeys();
	
	// Create a new WorldBounds reflecting the rotation in the world, this will make an AABB that will exceed the actual space, 
	// but will follow the defined bounds previously defined at author-time, but rotated.
	FVector Corners[8];
	InputData->CellDetails.Bounds.GetVertices(Corners);
	WorldBounds = FBox(ForceInit);
	for (const FVector& Corner : Corners)
	{
		WorldBounds += Rotation.RotateVector(Corner) + Position;
	}
	
	// Copy our hull data and rotate it into its new world-space position/rotation
	Hull = InputData->CellDetails.Hull;
	
	// Copy our voxel data and rotate it into its new world-space position/rotation
	WorldVoxel = InputData->CellDetails.VoxelData;
	WorldVoxel.RotatedAroundPivot(Position, Rotation); // TODO: Implement
	
	// We need to copy all the template junction data into our own local copy of the details that we will manipulate
	for (int32 i = 0; i < FreeJunctionKeys.Num(); i++)
	{
		const int32 JunctionKey = FreeJunctionKeys[i];
		FNCellJunctionDetails& Details = WorldJunctions.Add(JunctionKey, InputData->Junctions[JunctionKey]);

		// Compose with quaternions - (was adding the Rotation previously, but this better?)
		Details.WorldRotation = (Rotation.Quaternion() * Details.WorldRotation.Quaternion()).Rotator();
		Details.WorldLocation = FNVectorUtils::RotatedAroundPivot(Details.WorldLocation + Position, Position, Rotation);
	}
	
	
}

bool FNProcGenGraphCellNode::HasOpenJunctions() const
{
	// TODO: some sort of thing will need to be added for requirements
	return FreeJunctionKeys.Num() > 0;
}

TMap<int32, FNCellJunctionDetails*> FNProcGenGraphCellNode::GetOpenJunctions()
{
	TMap<int32, FNCellJunctionDetails*> Junctions;
	const int32 FreeCount = FreeJunctionKeys.Num();
	for (int32 i = 0; i < FreeCount; i++)
	{
		uint32 Key = FreeJunctionKeys[i];
		Junctions.Add(Key, &WorldJunctions[Key]);
	}
	return Junctions;
}

const TMap<int32, FNCellJunctionDetails>& FNProcGenGraphCellNode::GetJunctions() const
{
	return WorldJunctions;
}

FNCellJunctionDetails* FNProcGenGraphCellNode::GetJunctionDetails(const int32 JunctionKey)
{
	if (!WorldJunctions.Contains(JunctionKey))
	{
		return nullptr;
	}
	return &WorldJunctions[JunctionKey];
}

FNProcGenGraphNode* FNProcGenGraphCellNode::GetLinkedNode(const int32 JunctionKey)
{
	if (!Links.Contains(JunctionKey))
	{
		return nullptr;
	}
	return Links[JunctionKey];
}

bool FNProcGenGraphCellNode::SearchForMatchingCellInputData(const FNVirtualCellData* InputData, const int32 MaxDepth) const
{
	if (InputData == nullptr) return false;
	if (InputDataPtr == InputData) return true;
	if (MaxDepth <= 0) return false;

	TSet<const FNProcGenGraphCellNode*> Visited;
	Visited.Add(this);

	TQueue<TPair<const FNProcGenGraphCellNode*, int32>> Frontier;
	Frontier.Enqueue({this, 0});

	TPair<const FNProcGenGraphCellNode*, int32> Current;
	while (Frontier.Dequeue(Current))
	{
		if (Current.Value >= MaxDepth) continue;

		for (const TPair<int32, FNProcGenGraphNode*>& Link : Current.Key->Links)
		{
			const FNProcGenGraphNode* Linked = Link.Value;
			if (Linked == nullptr || Linked->GetNodeType() != ENProcGenGraphNodeType::Cell) continue;

			const FNProcGenGraphCellNode* CellNode = static_cast<const FNProcGenGraphCellNode*>(Linked);
			if (Visited.Contains(CellNode)) continue;
			Visited.Add(CellNode);

			if (CellNode->InputDataPtr == InputData) return true;

			Frontier.Enqueue({CellNode, Current.Value + 1});
		}
	}

	return false;
}

bool FNProcGenGraphCellNode::IsHullInside(const FBox& Bounds) const
{
	const TArray<FVector> WorldVertices = FNVectorUtils::RotateAndOffsetPoints(Hull.Vertices, GetWorldRotation(), GetWorldPosition());
	for (const FVector& Vertex : WorldVertices)
	{
		if (!Bounds.IsInside(Vertex))
		{
			return false;
		}
	}
	return WorldVertices.Num() > 0;
}

void FNProcGenGraphCellNode::Link(const int32 JunctionKey, FNProcGenGraphNode* Node)
{
	if (!FreeJunctionKeys.Contains(JunctionKey))
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("Cannot link junction key %d to node, key is not free"), JunctionKey);
		return;
	}
	
	if (InputDataPtr != nullptr)
	{
		InputDataPtr->UsedCount++;
	}
	FreeJunctionKeys.Remove(JunctionKey);
	Links.Add(JunctionKey, Node);
}

void FNProcGenGraphCellNode::Unlink(const int32 JunctionKey)
{
	if (!Links.Contains(JunctionKey))
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("Cannot unlink junction key %d from node, key is not linked"), JunctionKey);
		return;
	}
	
	if (InputDataPtr != nullptr)
	{
		InputDataPtr->UsedCount--;
	}
	Links.Remove(JunctionKey);
	FreeJunctionKeys.Add(JunctionKey);
}
