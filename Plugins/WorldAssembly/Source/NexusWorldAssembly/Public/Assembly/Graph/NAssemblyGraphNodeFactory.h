// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Data/NVirtualCellData.h"
#include "NAssemblyGraphBoneNode.h"
#include "NAssemblyGraphCellNode.h"
#include "NAssemblyGraphNullNode.h"

/**
 * Thin factory that allocates graph nodes with new, keeping ownership semantics explicit.
 *
 * Nodes returned from these helpers are handed to an FNAssemblyGraph via RegisterNode, which
 * then owns them for the remainder of the graph's lifetime.
 */
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory
class FNAssemblyGraphNodeFactory
{
public:

	/** Allocate a new bone node at the given transform, copying its socket size from Bone. */
	FORCEINLINE static FNAssemblyGraphBoneNode* CreateBoneNode(const FNVirtualBoneData* Bone, const FVector& Position, const FRotator& Rotation)
	{
		FNAssemblyGraphBoneNode* BoneNode = new FNAssemblyGraphBoneNode(Bone, Position, Rotation);
		return BoneNode;

	}

	/** Allocate a new cell node at the given transform, bound to the provided input data. */
	FORCEINLINE static FNAssemblyGraphCellNode* CreateCellNode(FNVirtualCellData* Cell, const FVector& Position, const FRotator& Rotation)
	{
		FNAssemblyGraphCellNode* CellNode = new FNAssemblyGraphCellNode(Cell, Position, Rotation);
		return CellNode;
	}

	/** Allocate a new null-terminator node at the given transform. */
	FORCEINLINE static FNAssemblyGraphNullNode* CreateNullNode(const FVector& Position, const FRotator& Rotation)
	{
		FNAssemblyGraphNullNode* NullNode = new FNAssemblyGraphNullNode(Position, Rotation);
		return NullNode;
	}
};
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory