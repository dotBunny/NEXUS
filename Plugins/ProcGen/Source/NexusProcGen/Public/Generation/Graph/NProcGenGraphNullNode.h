// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenGraphNode.h"

/**
 * Placeholder graph node that caps an open junction without spawning any content.
 *
 * Used when the builder needs to terminate a branch (e.g. blocked/closed junctions) while
 * keeping the graph topologically complete.
 */
class NEXUSPROCGEN_API FNProcGenGraphNullNode : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Null; }

	FNProcGenGraphNullNode(const FVector& Position, const FRotator& Rotation)
		: FNProcGenGraphNode(Position, Rotation)
	{
	}

	/** Link this terminator to Node. */
	void Link(FNProcGenGraphNode* Node);

	/** Drop the current link. */
	void Unlink();

private:
	/** The node this terminator is attached to; nullptr when free. */
	FNProcGenGraphNode* Linked = nullptr;
};