// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellInputData.h"
#include "NProcGenGraphNode.h"
#include "Cell/NCell.h"

class NEXUSPROCGEN_API FNProcGenGraphCellNode : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Cell; }
	
	FNProcGenGraphCellNode(FNCellInputData* InputData, const FVector& Position, const FRotator& Rotation);

	void ApplyJunctionsOffset(const FVector& Position, const FRotator& Rotation);
	
	void UpdateWorldPosition(const FVector& Position);
	void UpdateWorldRotation(const FRotator& Rotation);
	
	TMap<int32, FNCellJunctionDetails*> GetOpenJunctions();
	FNCellJunctionDetails* GetJunctionDetails(int32 JunctionKey);
	FNProcGenGraphNode* GetLinkedNode(int32 JunctionKey);
	
	TObjectPtr<UNCell> GetTemplate() const { return TemplatePtr; }
	
	void Link(int32 JunctionKey, FNProcGenGraphNode* Node);
	void Unlink(int32 JunctionKey);

private:
	
	FNCellInputData* InputDataPtr;
	
	TArray<int32> FreeJunctionKeys;
	TMap<int32, FNProcGenGraphNode*> Links;
	TMap<int32, FNCellJunctionDetails> WorldJunctions;
	
	TObjectPtr<UNCell> TemplatePtr;
};
