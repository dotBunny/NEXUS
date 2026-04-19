// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NCellInputData.h"
#include "NProcGenGraphNode.h"
#include "Cell/NCell.h"
#include "Types/NRawMeshUtils.h"

class NEXUSPROCGEN_API FNProcGenGraphCellNode : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Cell; }
	
	FNProcGenGraphCellNode(FNCellInputData* InputData, const FVector& Position, const FRotator& Rotation);
	
	bool HasOpenJunctions() const;
	TMap<int32, FNCellJunctionDetails*> GetOpenJunctions();
	const TMap<int32, FNCellJunctionDetails>& GetJunctions() const;
	
	FNCellJunctionDetails* GetJunctionDetails(int32 JunctionKey);
	FNProcGenGraphNode* GetLinkedNode(int32 JunctionKey);
	TObjectPtr<UNCell> GetTemplate() const { return TemplatePtr; }
	
	void Link(int32 JunctionKey, FNProcGenGraphNode* Node);
	void Unlink(int32 JunctionKey);
	
	bool CheckBoundsIntersects(const FNProcGenGraphCellNode* Other) const
	{
		return WorldBounds.Intersect(Other->WorldBounds);
	}
	
	bool IsBoundsInside(const FBox& Bounds) const
	{
		return Bounds.IsInside(WorldBounds);
	}
	
	bool IsHullInside(const FBox& Bounds) const;
	
	bool CheckHullIntersects(FNProcGenGraphCellNode* Other) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, GetWorldPosition(), GetWorldRotation(),
			Other->GetHull(), Other->GetWorldPosition(), Other->GetWorldRotation());
	}

protected:
	FNRawMesh& GetHull() { return Hull; }
	
private:
	
	FNCellInputData* InputDataPtr;
	TArray<int32> FreeJunctionKeys;
	TMap<int32, FNProcGenGraphNode*> Links;
	TMap<int32, FNCellJunctionDetails> WorldJunctions;
	
	FBox WorldBounds;
	FNRawMesh Hull;
	FNCellVoxelData WorldVoxel;
	TObjectPtr<UNCell> TemplatePtr;
};
