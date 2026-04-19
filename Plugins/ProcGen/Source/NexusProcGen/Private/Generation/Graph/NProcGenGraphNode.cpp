// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Graph/NProcGenGraphNode.h"

FNProcGenGraphNode::FNProcGenGraphNode(const FVector& Position, const FRotator& Rotation)
{
	WorldPosition = Position;
	WorldRotation = Rotation;
}

void FNProcGenGraphNode::SetWorldPosition(const FVector& Position)
{
	WorldPosition = Position;
}

void FNProcGenGraphNode::SetWorldRotation(const FRotator& Rotation)
{
	WorldRotation = Rotation;
}
