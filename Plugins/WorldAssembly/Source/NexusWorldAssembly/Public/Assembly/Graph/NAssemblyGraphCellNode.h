// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Data/NVirtualCellData.h"
#include "NAssemblyGraphNode.h"
#include "Cell/NCell.h"
#include "Types/NRawMeshUtils.h"

/**
 * Graph node representing a selected cell placed in world space.
 *
 * Tracks which junctions are free, which are linked (and to what), plus cached world-space
 * bounds/hull/voxel data used by intersection tests during graph expansion.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyGraphCellNode : public FNAssemblyGraphNode
{
public:
	virtual ENAssemblyGraphNodeType GetNodeType() const override {  return ENAssemblyGraphNodeType::Cell; }

	FNAssemblyGraphCellNode(FNVirtualCellData* InputData, uint64 CellSeed, const FVector& Position, const FRotator& Rotation, const FVector& VoxelSize);

	/** @return true if at least one junction on this cell is still unlinked. */
	bool HasOpenJunctions() const;

	/** @return Pointers into the cell's world-space junction map for every junction that is still free. */
	TMap<int32, FNCellJunctionDetails*> GetOpenJunctions();

	/** @return All of the cell's junctions in world-space. */
	const TMap<int32, FNCellJunctionDetails>& GetJunctions() const;

	/** @return Details for the junction with the given key, or nullptr if no such junction exists. */
	FNCellJunctionDetails* GetJunctionDetails(int32 JunctionKey);

	/** @return The graph node linked through the given junction, or nullptr if the junction is free. */
	FNAssemblyGraphNode* GetLinkedNode(int32 JunctionKey);

	/** @return The junction key whose link points to Node, or INDEX_NONE if not found. */
	int32 FindJunctionKeyLinkedTo(const FNAssemblyGraphNode* Node) const;

	/** @return The cell template this node was instantiated from. */
	TObjectPtr<UNCell> GetTemplate() const { return TemplatePtr; }

	/** Link the junction with the given key to Node — used by the builder when expanding connections. */
	void LinkJunction(int32 JunctionKey, FNAssemblyGraphNode* Node);

	/** Drop the link on the given junction, returning it to the free pool. */
	void UnlinkJunction(int32 JunctionKey);

	/** @return true if this cell's world bounds intersect Other's world bounds. */
	bool CheckBoundsIntersects(const FNAssemblyGraphCellNode* Other) const
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
	bool CheckHullIntersects(FNAssemblyGraphCellNode* Other) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, GetWorldPosition(), GetWorldRotation(),
			Other->GetHull(), Other->GetWorldPosition(), Other->GetWorldRotation());
	}
	
	/** @return true if this cell's hull intersects the supplied externally-transformed mesh. */
	bool CheckHullIntersects(const FVector& OtherLocation, const FRotator& OtherRotation,  const FNRawMesh& OtherMesh) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, GetWorldPosition(), GetWorldRotation(),
			OtherMesh, OtherLocation, OtherRotation);
	}

	/**
	 * @param Other Cell node whose hull is tested against this cell's hull.
	 * @param EarlyExitDepth Caller's threshold for the typical `if (depth >= threshold)` rejection pattern.
	 *        Passing the threshold lets FNRawMeshUtils::GetIntersectDepth short-circuit on either the
	 *        per-vertex deep-overlap path or the AABB-overlap upper bound — see that function's docs.
	 * @return true if this cell's hull intersects Other's hull, oriented by each cell's world transform.
	 */
	float GetHullIntersectDepth(FNAssemblyGraphCellNode* Other, float EarlyExitDepth = MAX_flt) const
	{
		return FNRawMeshUtils::GetIntersectDepth(Hull, GetWorldPosition(), GetWorldRotation(),
			Other->GetHull(), Other->GetWorldPosition(), Other->GetWorldRotation(), EarlyExitDepth);
	}

	/**
	 * @param EarlyExitDepth Caller's threshold for the typical `if (depth >= threshold)` rejection pattern.
	 *        See FNRawMeshUtils::GetIntersectDepth for the semantics of the short-circuit shortcuts.
	 * @return The deepest penetration distance between this cell's hull and the supplied externally-transformed mesh,
	 *         or -1.0 when their AABBs do not overlap or the meshes are otherwise unmeasurable (see
	 *         FNRawMeshUtils::GetIntersectDepth for the full set of sentinel cases).
	 * @note Intended for "max-allowed-penetration" threshold checks during graph expansion; use CheckHullIntersects
	 *       for an exact boolean overlap test.
	 */
	float GetHullIntersectDepth(const FVector& OtherLocation, const FRotator& OtherRotation, const FNRawMesh& OtherMesh, const float EarlyExitDepth = MAX_flt) const
	{
		return FNRawMeshUtils::GetIntersectDepth(Hull, GetWorldPosition(), GetWorldRotation(),
			OtherMesh, OtherLocation, OtherRotation, EarlyExitDepth);
	}

	/**
	 * @return The raw input-data pointer used by the builder.
	 * @warning Only valid during graph construction; nulled by CleanupBuilderReferences afterward.
	 */
	FNVirtualCellData* GetInputDataPtr() const { return InputDataPtr; }

	/** Drop the builder-only input-data pointer once the graph is handed to the next stage. */
	void CleanupBuilderReferences()
	{
		InputDataPtr = nullptr;
	}

	/** @return Mutable access to the cell's world-space hull for intersection tests. */
	FNRawMesh& GetHull() { return Hull; }
	/** @return A by-value copy of the cell's hull for callers that need an independent mesh. */
	FNRawMesh GetHullCopy() { return Hull; }
	
	bool HasAssemblyTags() const { return !AssemblyTags.IsEmpty(); }

private:
	/** Non-owning pointer to the input data this cell was chosen from; only valid during builder phase. */
	FNVirtualCellData* InputDataPtr;
	
	/** Whether the spawned level instance should be always-relevant for networking. */
	bool bAlwaysRelevant;

	/** Junction keys that are still unlinked. */
	TArray<int32> FreeJunctionKeys;

	/** Map of junction key -> linked node (populated via Link/Unlink). */
	TMap<int32, FNAssemblyGraphNode*> Links;

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
