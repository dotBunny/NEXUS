// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraphCellNode.h"

#include "NWorldAssemblyMinimal.h"
#include "Math/NVectorUtils.h"

FNAssemblyGraphCellNode::FNAssemblyGraphCellNode(const FNAssemblyGraphNodeParams& Params, FNVirtualCellData* InputData, const FVector& VoxelSize) 
: FNAssemblyGraphNode(Params)
{
	// Copy InputData to disconnect from reference
	InputDataPtr = InputData;
	TemplatePtr = InputDataPtr->Template; // Might not need in future
	FreeJunctionKeys = InputDataPtr->GetJunctionKeys();
	
	// Create a new WorldBounds reflecting the rotation in the world, this will make an AABB that will exceed the actual space, 
	// but will follow the defined bounds previously defined at author-time, but rotated.
	TStaticArray<FVector, 8> Corners;
	InputData->CellDetails.Bounds.GetVertices(Corners.Elements);
	WorldBounds = FBox(ForceInit);
	for (const FVector& Corner : Corners)
	{
		WorldBounds += Params.WorldRotation.RotateVector(Corner) + Params.WorldPosition;
	}
	
	// Copy our hull data and rotate it into its new world-space position/rotation
	Hull = InputData->CellDetails.Hull;
	
	// Bake the hull position
	FTransform WorldTransform(Params.WorldRotation, Params.WorldPosition);
	Hull.ApplyTransform(WorldTransform);
	
	// Cache optimized collision data for convex hulls ahead of any checks
	if (Hull.IsConvex())
	{
		Hull.EnsureCachedFacePlanes();
	}
	
	// Copy our voxel data and rotate it into its new world-space position/rotation
	// TODO: Right now we dont actually use the VoxelData for anything so lets not pay for the rotation until we need it, not assigning the data to cause an error later so we know.
	//WorldVoxel = InputData->CellDetails.VoxelData;
	//WorldVoxel.RotateAroundPivot(Position, Rotation, VoxelSize);
	
	// We need to copy all the template junction data into our own local copy of the details that we will manipulate
	for (int32 i = 0; i < FreeJunctionKeys.Num(); i++)
	{
		const int32 JunctionKey = FreeJunctionKeys[i];
		FNCellJunctionDetails& Details = WorldJunctions.Add(JunctionKey, InputData->Junctions[JunctionKey]);

		// Compose with quaternions - (was adding the Rotation previously, but this better?)
		Details.WorldRotation = (Params.WorldRotation.Quaternion() * Details.WorldRotation.Quaternion()).Rotator();
		Details.WorldLocation = FNVectorUtils::RotatedAroundPivot(Details.WorldLocation + Params.WorldPosition, 
			Params.WorldPosition, Params.WorldRotation);
	}
}

bool FNAssemblyGraphCellNode::HasOpenJunctions() const
{
	// Reports whether any junction is free at all; for the Required-only variant the builder/validator wants,
	// see FindUnconnectedRequiredJunctionKey.
	return FreeJunctionKeys.Num() > 0;
}

int32 FNAssemblyGraphCellNode::FindUnconnectedRequiredJunctionKey() const
{
	// FreeJunctionKeys holds exactly the unconnected junctions, so a Required junction appearing here means the
	// graph left a mandatory connection open.
	for (const int32 JunctionKey : FreeJunctionKeys)
	{
		if (WorldJunctions[JunctionKey].Requirements == ENCellJunctionRequirements::Required)
		{
			return JunctionKey;
		}
	}
	return INDEX_NONE;
}

TMap<int32, FNCellJunctionDetails*> FNAssemblyGraphCellNode::GetOpenJunctions()
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
const TMap<int32, FNCellJunctionDetails>& FNAssemblyGraphCellNode::GetJunctions() const
{
	return WorldJunctions;
}

FNCellJunctionDetails* FNAssemblyGraphCellNode::GetJunctionDetails(const int32 JunctionKey)
{
	if (!WorldJunctions.Contains(JunctionKey))
	{
		return nullptr;
	}
	return &WorldJunctions[JunctionKey];
}

FNAssemblyGraphNode* FNAssemblyGraphCellNode::GetLinkedNode(const int32 JunctionKey)
{
	if (!Links.Contains(JunctionKey))
	{
		return nullptr;
	}
	return Links[JunctionKey];
}

bool FNAssemblyGraphCellNode::SearchForMatchingCellInputData(const FNVirtualCellData* InputData, const int32 MaxDepth) const
{
	if (InputData == nullptr) return false;
	if (InputDataPtr == InputData) return true;
	if (MaxDepth <= 0) return false;

	TSet<const FNAssemblyGraphCellNode*> Visited;
	Visited.Add(this);

	TQueue<TPair<const FNAssemblyGraphCellNode*, int32>> Frontier;
	Frontier.Enqueue({this, 0});

	TPair<const FNAssemblyGraphCellNode*, int32> Current;
	while (Frontier.Dequeue(Current))
	{
		if (Current.Value >= MaxDepth) continue;

		for (const TPair<int32, FNAssemblyGraphNode*>& Link : Current.Key->Links)
		{
			const FNAssemblyGraphNode* Linked = Link.Value;
			if (Linked == nullptr || Linked->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

			const FNAssemblyGraphCellNode* CellNode = static_cast<const FNAssemblyGraphCellNode*>(Linked);
			if (Visited.Contains(CellNode)) continue;
			Visited.Add(CellNode);

			if (CellNode->InputDataPtr == InputData) return true;

			Frontier.Enqueue({CellNode, Current.Value + 1});
		}
	}

	return false;
}

bool FNAssemblyGraphCellNode::IsHullInside(const FBox& Bounds) const
{
	// Hull is already baked into world space at construction, so test its vertices directly.
	for (const FVector& Vertex : Hull.Vertices)
	{
		if (!Bounds.IsInside(Vertex))
		{
			return false;
		}
	}
	return Hull.Vertices.Num() > 0;
}

void FNAssemblyGraphCellNode::GenerateLinkDetails()
{
	if (LinkDetails.Num() > 0) return;

	// One entry per junction, in WorldJunctions map order so it stays aligned with the JunctionDetails
	// array the spawn task generates from GetJunctions().
	LinkDetails.Reserve(WorldJunctions.Num());
	for (const TPair<int32, FNCellJunctionDetails>& Junction : WorldJunctions)
	{
		FNCellLinkDetails& Details = LinkDetails.AddDefaulted_GetRef();
		Details.JunctionInstanceIdentifier = Junction.Value.InstanceIdentifier;

		// A junction that is still free has no link, leave the connection fields at their defaults.
		FNAssemblyGraphNode** LinkedNodePtr = Links.Find(Junction.Key);
		if (LinkedNodePtr == nullptr) continue;

		FNAssemblyGraphNode* LinkedNode = *LinkedNodePtr;
		Details.bConnected = true;
		Details.ConnectedNodeIdentifier = LinkedNode->GetNodeIdentifier();

		// Only cell nodes carry junctions; resolve the junction on the far cell that links back to us.
		if (LinkedNode->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

		FNAssemblyGraphCellNode* LinkedCellNode = static_cast<FNAssemblyGraphCellNode*>(LinkedNode);
		const int32 ConnectedJunctionKey = LinkedCellNode->FindJunctionKeyLinkedTo(this);
		if (const FNCellJunctionDetails* ConnectedJunction = LinkedCellNode->GetJunctionDetails(ConnectedJunctionKey))
		{
			Details.ConnectedJunctionInstanceIdentifier = ConnectedJunction->InstanceIdentifier;
		}

		// The junction lies along a hot path when both this cell and the cell it links to are on it.
		Details.bHotPathShortest = bHotPathShortest && LinkedCellNode->bHotPathShortest;
		Details.bHotPathSequential = bHotPathSequential && LinkedCellNode->bHotPathSequential;
	}
}

void FNAssemblyGraphCellNode::LinkJunction(const int32 JunctionKey, FNAssemblyGraphNode* Node)
{
	if (!FreeJunctionKeys.Contains(JunctionKey))
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("Cannot link junction key %d to node, key is not free"), JunctionKey);
		return;
	}
	
	FreeJunctionKeys.Remove(JunctionKey);
	Links.Add(JunctionKey, Node);
}

int32 FNAssemblyGraphCellNode::FindJunctionKeyLinkedTo(const FNAssemblyGraphNode* Node) const
{
	for (const auto& Link : Links)
	{
		if (Link.Value == Node)
		{
			return Link.Key;
		}
	}
	return INDEX_NONE;
}

void FNAssemblyGraphCellNode::UnlinkJunction(const int32 JunctionKey)
{
	if (!Links.Contains(JunctionKey))
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("Cannot unlink junction key %d from node, key is not linked"), JunctionKey);
		return;
	}
	
	Links.Remove(JunctionKey);
	FreeJunctionKeys.Add(JunctionKey);
}
