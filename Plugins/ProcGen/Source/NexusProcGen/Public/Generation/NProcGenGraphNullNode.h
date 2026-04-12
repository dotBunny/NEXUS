// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenGraphNode.h"

class NEXUSPROCGEN_API FNProcGenGraphNullNode : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Null; }
	
	FNProcGenGraphNullNode(const FVector& Position, const FRotator& Rotation)
		: FNProcGenGraphNode(Position, Rotation)
	{
	}
	
	void Link(FNProcGenGraphNode* Node);
	void Unlink();
	
private:
	FNProcGenGraphNode* Linked = nullptr;
};