// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Data/NVirtualCellData.h"
#include "NProcGenGraphNode.h"
#include "Cell/NCell.h"
#include "Types/NRawMeshUtils.h"

/**
 * Graph node representing a selected cell placed in world space.
 *
 * Tracks which junctions are free, which are linked (and to what), plus cached world-space
 * bounds/hull/voxel data used by intersection tests during graph expansion.
 */
class NEXUSPROCGEN_API FNProcGenGraphCellNode : public FNProcGenGraphNode
{
public:
	virtual ENProcGenGraphNodeType GetNodeType() const override {  return ENProcGenGraphNodeType::Cell; }

	FNProcGenGraphCellNode(FNVirtualCellData* InputData, const FVector& Position, const FRotator& Rotation);

	/** @return true if at least one junction on this cell is still unlinked. */
	bool HasOpenJunctions() const;

	/** @return Pointers into the cell's world-space junction map for every junction that is still free. */
	TMap<int32, FNCellJunctionDetails*> GetOpenJunctions();

	/** @return All of the cell's junctions in world-space. */
	const TMap<int32, FNCellJunctionDetails>& GetJunctions() const;

	/** @return Details for the junction with the given key, or nullptr if no such junction exists. */
	FNCellJunctionDetails* GetJunctionDetails(int32 JunctionKey);

	/** @return The graph node linked through the given junction, or nullptr if the junction is free. */
	FNProcGenGraphNode* GetLinkedNode(int32 JunctionKey);

	/** @return The cell template this node was instantiated from. */
	TObjectPtr<UNCell> GetTemplate() const { return TemplatePtr; }

	/** Link the junction with the given key to Node — used by the builder when expanding connections. */
	void Link(int32 JunctionKey, FNProcGenGraphNode* Node);

	/** Drop the link on the given junction, returning it to the free pool. */
	void Unlink(int32 JunctionKey);

	/** @return true if this cell's world bounds intersect Other's world bounds. */
	bool CheckBoundsIntersects(const FNProcGenGraphCellNode* Other) const
	{
		return WorldBounds.Intersect(Other->WorldBounds);
	}
	


	/**
	 * Walks linked neighbours up to MaxDepth looking for an existing node built from the same input data.
	 * Used to enforce MinimumNodeDistance constraints.
	 */
	bool SearchForMatchingCellInputData(const FNVirtualCellData* InputData, int32 MaxDepth) const;

	/** @return true if the node's world bounds are fully contained within Bounds. */
	bool IsBoundsInside(const FBox& Bounds) const
	{
		return Bounds.IsInside(WorldBounds);
	}

	/** @return true if every hull vertex of the node lies inside Bounds. */
	bool IsHullInside(const FBox& Bounds) const;

	/** @return true if this cell's hull intersects Other's hull, oriented by each cell's world transform. */
	bool CheckHullIntersects(FNProcGenGraphCellNode* Other) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, GetWorldPosition(), GetWorldRotation(),
			Other->GetHull(), Other->GetWorldPosition(), Other->GetWorldRotation());
	}
	
	bool CheckHullIntersects(const FVector& OtherLocation, const FRotator& OtherRotation,  const FNRawMesh& OtherMesh) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, GetWorldPosition(), GetWorldRotation(),
			OtherMesh, OtherLocation, OtherRotation);
	}

	/**
	 * @return The raw input-data pointer used by the builder.
	 * @warning Only valid during graph construction; nulled by CleanupBuilderReferences afterwards.
	 */
	FNVirtualCellData* GetInputDataPtr() const { return InputDataPtr; }

	/** Drop the builder-only input-data pointer once the graph is handed to the next stage. */
	void CleanupBuilderReferences()
	{
		InputDataPtr = nullptr;
	}

	/** @return Mutable access to the cell's world-space hull for intersection tests. */
	FNRawMesh& GetHull() { return Hull; }
	FNRawMesh GetHullCopy() { return Hull; }

private:
	/** Non-owning pointer to the input data this cell was chosen from; only valid during builder phase. */
	FNVirtualCellData* InputDataPtr;

	/** Whether the spawned level instance should be always-relevant for networking. */
	bool bAlwaysRelevant;

	/** Junction keys that are still unlinked. */
	TArray<int32> FreeJunctionKeys;

	/** Map of junction key -> linked node (populated via Link/Unlink). */
	TMap<int32, FNProcGenGraphNode*> Links;

	/** Cell junctions transformed into world space. */
	TMap<int32, FNCellJunctionDetails> WorldJunctions;

	/** World-space axis-aligned bounds used for broad-phase intersection tests. */
	FBox WorldBounds;

	/** Cell hull in local space; oriented into world space by GetWorldPosition/Rotation. */
	FNRawMesh Hull;

	/** Cell voxel data transformed into the graph's world space. */
	FNCellVoxelData WorldVoxel;

	/** Resolved template pointer; kept alive through the graph's lifetime. */
	TObjectPtr<UNCell> TemplatePtr;
};
