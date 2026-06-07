// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraphNode.h"

FNAssemblyGraphNode::FNAssemblyGraphNode(const uint64 NodeSeed, const FVector& Position, const FRotator& Rotation)
{
	Seed = NodeSeed;
	WorldPosition = Position;
	WorldRotation = Rotation;
}

FNAssemblyGraphNode::FNAssemblyGraphNode(const FNAssemblyGraphNodeParams& Params)
{
	Seed = Params.Seed;
	
	WorldPosition = Params.WorldPosition;
	WorldRotation = Params.WorldRotation;
	
	ContextTagsAdded = Params.ContextTagsAdded;
	ContextTagsState = Params.ContextTagsState;
	AssemblyTags = Params.AssemblyTags;
	TagCounterState = Params.TagCounterState;
}

void FNAssemblyGraphNode::SetWorldPosition(const FVector& Position)
{
	WorldPosition = Position;
}

void FNAssemblyGraphNode::SetWorldRotation(const FRotator& Rotation)
{
	WorldRotation = Rotation;
}
