// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Data/NVirtualBoneData.h"
#include "NAssemblyGraphNode.h"

/**
 * Graph node representing a bone — a pre-placed junction anchor that cell nodes attach to.
 *
 * A bone node owns a single socket and at most one linked neighbour; the graph builder uses
 * it as a fixed attachment point when expanding outward from the organ's skeleton.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyGraphBoneNode  : public FNAssemblyGraphNode
{
public:
	virtual ENAssemblyGraphNodeType GetNodeType() const override {  return ENAssemblyGraphNodeType::Bone; }
	FNAssemblyGraphBoneNode(const FNVirtualBoneData* InputData, const FVector& Position, const FRotator& Rotation)
	: FNAssemblyGraphNode(0, Position, Rotation) // No need for seed atm
	{
		SocketSize = InputData->SocketSize;
		NodeDepth = 0;
	}

	/** Link the bone's socket to Node. */
	void Link(FNAssemblyGraphNode* Node);

	/** Drop the bone's current link, returning the socket to the free pool. */
	void Unlink();

private:
	/** Bone socket size in grid units, copied from the source FNVirtualBoneData. */
	FIntVector2 SocketSize;

	/** The node currently linked to this bone; nullptr when free. */
	FNAssemblyGraphNode* Linked = nullptr;
};
