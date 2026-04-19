// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NBoneInputData.h"
#include "NProcGenGraphNode.h"

class NEXUSPROCGEN_API FNProcGenGraphBoneNode  : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Bone; }
	FNProcGenGraphBoneNode(const FNBoneInputData* InputData, const FVector& Position, const FRotator& Rotation) 
	: FNProcGenGraphNode(Position, Rotation)
	{
		SocketSize = InputData->SocketSize;
	}
	
	void Link(FNProcGenGraphNode* Node);
	void Unlink();
	
private:
	FIntVector2 SocketSize;
	FNProcGenGraphNode* Linked = nullptr;
};
