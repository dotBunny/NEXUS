// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraphCellNode.h"

#include "NProcGenMinimal.h"
#include "Math/NVectorUtils.h"

FNProcGenGraphCellNode::FNProcGenGraphCellNode(FNCellInputData* InputData, const FVector& Position, const FRotator& Rotation) : FNProcGenGraphNode(Position, Rotation)
{
	InputDataPtr = InputData;
	TemplatePtr = InputDataPtr->Template; // Might not need in future
	FreeJunctionKeys = InputDataPtr->GetJunctionKeys();
	
	// Copy bounds and rotate into their new world-space position/rotation
	WorldBounds = InputData->CellDetails.Bounds;
	WorldBounds.Min = FNVectorUtils::RotatedAroundPivot(WorldBounds.Min + Position, Position, Rotation);
	WorldBounds.Max = FNVectorUtils::RotatedAroundPivot(WorldBounds.Max + Position, Position, Rotation);
	
	// Copy our hull data and rotate it into its new world-space position/rotation
	Hull = InputData->CellDetails.Hull;
	//WorldHull.RotatedAroundPivot(Position, Rotation); // dont think we need to move these as they are local and the comparison is in local space
	
	// Copy our voxel data and rotate it into its new world-space position/rotation
	WorldVoxel = InputData->CellDetails.VoxelData;
	WorldVoxel.RotatedAroundPivot(Position, Rotation); // TODO: Implement
	
	// We need to copy all the template junction data into our own local copy of the details that we will manipulate
	for (int i = 0; i < FreeJunctionKeys.Num(); i++)
	{
		const int32 JunctionKey = FreeJunctionKeys[i];
		FNCellJunctionDetails& Details = WorldJunctions.Add(JunctionKey, InputData->Junctions[JunctionKey]);
		
		// Update position/rotation to reflect actual world placement
		Details.WorldRotation += Rotation;
		Details.WorldLocation = FNVectorUtils::RotatedAroundPivot(Details.WorldLocation + Position, Position, Rotation);
	}
	
	
}

void FNProcGenGraphCellNode::UpdateWorldPosition(const FVector& Position)
{
	// TODO: Shift links?
	SetWorldPosition(Position);
}

void FNProcGenGraphCellNode::UpdateWorldRotation(const FRotator& Rotation)
{
	// TODO: Shift links?
	SetWorldRotation(Rotation);
}

TMap<int32, FNCellJunctionDetails*> FNProcGenGraphCellNode::GetOpenJunctions()
{
	TMap<int32, FNCellJunctionDetails*> Junctions;
	const int FreeCount = FreeJunctionKeys.Num();
	for (int i = 0; i < FreeCount; i++)
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
