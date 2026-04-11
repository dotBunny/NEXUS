// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NCellGraphNode.h"

void FNCellGraphNode::UpdateWorldPosition(const FVector& Position)
{
	// TODO: Shift links?
	WorldPosition = Position;
}

void FNCellGraphNode::UpdateWorldRotation(const FRotator& Rotation)
{
	// TODO: Shift links?
	WorldRotation = Rotation;
}
