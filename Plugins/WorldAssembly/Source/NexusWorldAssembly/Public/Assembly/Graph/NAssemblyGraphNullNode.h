// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NAssemblyGraphNode.h"

/**
 * Placeholder graph node that caps an open junction without spawning any content.
 *
 * Used when the builder needs to terminate a branch (e.g. blocked/closed junctions) while
 * keeping the graph topologically complete.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyGraphNullNode : public FNAssemblyGraphNode
{
public:
	virtual ENAssemblyGraphNodeType GetNodeType() const override {  return ENAssemblyGraphNodeType::Null; }

	FNAssemblyGraphNullNode(const FVector& Position, const FRotator& Rotation)
		: FNAssemblyGraphNode(0, Position, Rotation) // no need for seed atm
	{
	}

	/** Link this terminator to Node. */
	void Link(FNAssemblyGraphNode* Node);

	/** Drop the current link. */
	void Unlink();

private:
	/** The node this terminator is attached to; nullptr when free. */
	FNAssemblyGraphNode* Linked = nullptr;
};