// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraphCellNode.h"

#include "NProcGenMinimal.h"
#include "Math/NVectorUtils.h"

FNProcGenGraphCellNode::FNProcGenGraphCellNode(FNCellInputData* InputData, const FVector& Position, const FRotator& Rotation) : FNProcGenGraphNode(Position, Rotation)
{
	InputData->UsedCount++;

	TemplatePtr = InputData->Template;
	FreeJunctionKeys = InputData->GetJunctionKeys();
	
	// We need to copy all the template junction data into our own local copy of the details that we will manipulate
	for (int i = 0; i < FreeJunctionKeys.Num(); i++)
	{
		const int32 JunctionKey = FreeJunctionKeys[i];
		FNCellJunctionDetails& Details = WorldJunctions.Add(JunctionKey, InputData->Junctions[JunctionKey]);
		
		// Update position/rotation to reflect actual world placement
		Details.RootRelativeRotation += Rotation;
		Details.RootRelativeLocation = FNVectorUtils::RotatedAroundPivot(Details.RootRelativeLocation + Position, Position, Rotation);
	}
	
	InputDataPtr = InputData;
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
	Links.Remove(JunctionKey);
	FreeJunctionKeys.Add(JunctionKey);
}
