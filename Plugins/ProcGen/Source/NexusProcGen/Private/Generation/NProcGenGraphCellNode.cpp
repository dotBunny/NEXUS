// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraphCellNode.h"

#include "NProcGenMinimal.h"

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
		Junctions.Add(FreeJunctionKeys[i], &WorldJunctions[FreeJunctionKeys[i]]);
	}
	return Junctions;
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
