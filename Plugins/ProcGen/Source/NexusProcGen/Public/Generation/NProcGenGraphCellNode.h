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
		
		// We need to copy all the template junction data into our own local copy of the details that we will manipulate
		for (int i = 0; i < FreeJunctionKeys.Num(); i++)
		{
			const int JunctionKey = FreeJunctionKeys[i];
			WorldJunctions.Add(FreeJunctionKeys[i], InputData->Junctions[JunctionKey]);
			
			// We're going to manipulate the data bit to cache future calculations
			// TODO: We need to ...draw these?
			FNCellJunctionDetails& JunctionDetails = WorldJunctions.FindChecked(FreeJunctionKeys[i]);
			JunctionDetails.RootRelativeRotation = JunctionDetails.RootRelativeRotation + Rotation;
			JunctionDetails.RootRelativeLocation = JunctionDetails.RootRelativeLocation + Position;
		}

		// TODO: Unsure if we want to have this ref
		InputDataPtr = InputData;
	}

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
