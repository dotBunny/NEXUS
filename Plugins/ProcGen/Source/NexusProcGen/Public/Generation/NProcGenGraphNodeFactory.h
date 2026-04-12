// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NCellInputData.h"
#include "NProcGenGraphBoneNode.h"
#include "NProcGenGraphCellNode.h"
#include "NProcGenGraphNullNode.h"

// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory
class FNProcGenGraphNodeFactory
{
public:
	
	FORCEINLINE static FNProcGenGraphBoneNode* CreateBoneNode(const FNBoneInputData* Bone, const FVector& Position, const FRotator& Rotation)
	{
		FNProcGenGraphBoneNode* BoneNode = new FNProcGenGraphBoneNode(Bone, Position, Rotation);
		return BoneNode;
		
	}
	FORCEINLINE static FNProcGenGraphCellNode* CreateCellNode(FNCellInputData* Cell, const FVector& Position, const FRotator& Rotation)
	{
		FNProcGenGraphCellNode* CellNode = new FNProcGenGraphCellNode(Cell, Position, Rotation);
		return CellNode;
	}
	
	FORCEINLINE static FNProcGenGraphNullNode* CreateNullNode(const FVector& Position, const FRotator& Rotation)
	{
		FNProcGenGraphNullNode* NullNode = new FNProcGenGraphNullNode(Position, Rotation);
		return NullNode;
	}
};
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory