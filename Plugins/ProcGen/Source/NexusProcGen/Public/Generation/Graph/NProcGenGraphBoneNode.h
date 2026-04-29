// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Data/NVirtualBoneData.h"
#include "NProcGenGraphNode.h"

/**
 * Graph node representing a bone — a pre-placed junction anchor that cell nodes attach to.
 *
 * A bone node owns a single socket and at most one linked neighbour; the graph builder uses
 * it as a fixed attachment point when expanding outward from the organ's skeleton.
 */
class NEXUSPROCGEN_API FNProcGenGraphBoneNode  : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Bone; }
	FNProcGenGraphBoneNode(const FNVirtualBoneData* InputData, const FVector& Position, const FRotator& Rotation)
	: FNProcGenGraphNode(Position, Rotation)
	{
		SocketSize = InputData->SocketSize;
	}

	/** Link the bone's socket to Node. */
	void Link(FNProcGenGraphNode* Node);

	/** Drop the bone's current link, returning the socket to the free pool. */
	void Unlink();

private:
	/** Bone socket size in grid units, copied from the source FNBoneInputData. */
	FIntVector2 SocketSize;

	/** The node currently linked to this bone; nullptr when free. */
	FNProcGenGraphNode* Linked = nullptr;
};
