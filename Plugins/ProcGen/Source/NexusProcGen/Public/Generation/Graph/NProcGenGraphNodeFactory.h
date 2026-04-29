// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Data/NVirtualCellData.h"
#include "NProcGenGraphBoneNode.h"
#include "NProcGenGraphCellNode.h"
#include "NProcGenGraphNullNode.h"

/**
 * Thin factory that allocates graph nodes with new, keeping ownership semantics explicit.
 *
 * Nodes returned from these helpers are handed to an FNProcGenGraph via RegisterNode, which
 * then owns them for the remainder of the graph's lifetime.
 */
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory
class FNProcGenGraphNodeFactory
{
public:

	/** Allocate a new bone node at the given transform, copying its socket size from Bone. */
	FORCEINLINE static FNProcGenGraphBoneNode* CreateBoneNode(const FNVirtualBoneData* Bone, const FVector& Position, const FRotator& Rotation)
	{
		FNProcGenGraphBoneNode* BoneNode = new FNProcGenGraphBoneNode(Bone, Position, Rotation);
		return BoneNode;

	}

	/** Allocate a new cell node at the given transform, bound to the provided input data. */
	FORCEINLINE static FNProcGenGraphCellNode* CreateCellNode(FNVirtualCellData* Cell, const FVector& Position, const FRotator& Rotation)
	{
		FNProcGenGraphCellNode* CellNode = new FNProcGenGraphCellNode(Cell, Position, Rotation);
		return CellNode;
	}

	/** Allocate a new null-terminator node at the given transform. */
	FORCEINLINE static FNProcGenGraphNullNode* CreateNullNode(const FVector& Position, const FRotator& Rotation)
	{
		FNProcGenGraphNullNode* NullNode = new FNProcGenGraphNullNode(Position, Rotation);
		return NullNode;
	}
};
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory