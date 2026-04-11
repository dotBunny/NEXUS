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
	
	FNProcGenGraphCellNode(FNCellInputData* InputData, const FVector& Position, const FRotator& Rotation)
		: FNProcGenGraphNode(Position, Rotation)
	{
		InputData->UsedCount++;

		TemplatePtr = InputData->Template;
		FreeJunctionKeys = InputData->GetJunctionKeys();

		// Unsure if we want to have this ref
		InputDataPtr = InputData;
	}

	void UpdateWorldPosition(const FVector& Position);
	void UpdateWorldRotation(const FRotator& Rotation);
	

	TObjectPtr<UNCell> GetTemplate() const { return TemplatePtr; }
	
	void Link(int32 JunctionKey, FNProcGenGraphNode* Node);
	void Unlink(int32 JunctionKey);

private:
	
	FNCellInputData* InputDataPtr;
	//FNCellJunctionDetails
	
	TArray<int32> FreeJunctionKeys;
	TMap<int32, FNProcGenGraphNode*> Links;
	
	// MAIN-THREAD USE ONLY // ROOT OBJECT NEEDS TO BE UNLOADED LATER
	TObjectPtr<UNCell> TemplatePtr;
};
