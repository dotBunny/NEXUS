// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Data/NVirtualCellData.h"
#include "NAssemblyGraphNode.h"
#include "Cell/NCell.h"
#include "Cell/NCellLinkDetails.h"
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

	FNAssemblyGraphCellNode(const FNAssemblyGraphNodeParams& Params, FNVirtualCellData* InputData, const FVector& VoxelSize);

	/** @return true if at least one junction on this cell is still unlinked. */
	bool HasOpenJunctions() const;

	/**
	 * @return The key of the first junction flagged ENCellJunctionRequirements::Required that is still unconnected
	 *         (in the free pool), or INDEX_NONE when every Required junction has been linked.
	 * @note Unlike HasOpenJunctions, which reports whether any junction at all is free, this only considers
	 *       junctions whose Requirements demand a connection; enforced by FNVirtualOrganContext::CheckGraph.
	 */
	int32 FindUnconnectedRequiredJunctionKey() const;

	/** @return Pointers into the cell's world-space junction map for every junction that is still free. */
	TMap<int32, FNCellJunctionDetails*> GetOpenJunctions();

	/** @return All of the cell's junctions in world-space. */
	const TMap<int32, FNCellJunctionDetails>& GetJunctions() const;

	const TArray<FNCellLinkDetails>& GetLinkDetails() const { return LinkDetails; }

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

	/**
	 * Link the junction with the given key to Node as a connector pairing — two junctions the connector pass matched
	 * up across open space rather than two cells mating directly.
	 *
	 * Behaves as LinkJunction, and additionally records ConnectorIdentifier so GenerateLinkDetails can stamp it (and
	 * the bConnector flag) onto the junction's link details.
	 * @param JunctionKey The junction on this cell being paired.
	 * @param Node The cell node on the far end of the pairing.
	 * @param ConnectorIdentifier Operation-unique identifier of the pairing; the same value is recorded on both ends.
	 * @note Called on both ends of a pairing. It deliberately does not wire the node-level Connect edge that makes
	 *       the pairing visible to graph traversal — that edge is directional and belongs to the pair, not to one
	 *       junction, so the caller makes a single Connect call from its chosen start node. See FNConnectJunctionsTask.
	 */
	void LinkJunctionConnector(int32 JunctionKey, FNAssemblyGraphNode* Node, int32 ConnectorIdentifier);

	/** Drop the link on the given junction, returning it to the free pool. */
	void UnlinkJunction(int32 JunctionKey);

	/**
	 * @return World-space center of the cell's (rotation-baked) bounds. Differs from GetWorldPosition when the
	 *         authored bounds are not centered on the cell pivot.
	 */
	FVector GetWorldBoundsCenter() const { return WorldBounds.GetCenter(); }

	/** @return true if this cell's world bounds intersect Other's world bounds. */
	bool CheckBoundsIntersects(const FNAssemblyGraphCellNode* Other) const
	{
		return WorldBounds.Intersect(Other->WorldBounds);
	}

	/** @return true if this cell's world bounds intersect Other. */
	bool CheckBoundsIntersects(const FBox& Other) const
	{
		return WorldBounds.Intersect(Other);
	}

	/**
	 * @return The cell's rotation-baked world-space AABB — the broadphase key for cell-vs-cell bounds queries.
	 * @note Set at construction and never mutated, which is what lets FNAssemblyGraph index these in a tree that
	 *       stays valid as long as the node is registered.
	 */
	const FBox& GetWorldBounds() const { return WorldBounds; }

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

	/** @return true if this cell's (world-space baked) hull intersects Other's (world-space baked) hull. */
	bool CheckHullIntersects(FNAssemblyGraphCellNode* Other) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, Other->GetHull());
	}

	/** @return true if this cell's (world-space baked) hull intersects the supplied world-space mesh. */
	bool CheckHullIntersects(const FNRawMesh& OtherMesh) const
	{
		return FNRawMeshUtils::DoesIntersect(Hull, OtherMesh);
	}

	/**
	 * @param Other Cell node whose hull is tested against this cell's hull.
	 * @param EarlyExitDepth Caller's threshold for the typical `if (depth >= threshold)` rejection pattern.
	 *        Passing the threshold lets FNRawMeshUtils::GetIntersectDepth short-circuit on either the
	 *        per-vertex deep-overlap path or the AABB-overlap upper bound — see that function's docs.
	 * @return The deepest penetration distance between this cell's (world-space baked) hull and Other's
	 *         (world-space baked) hull. See the mesh overload for sentinel semantics.
	 */
	float GetHullIntersectDepth(FNAssemblyGraphCellNode* Other, float EarlyExitDepth = MAX_flt) const
	{
		return FNRawMeshUtils::GetIntersectDepth(Hull, Other->GetHull(), EarlyExitDepth);
	}

	/**
	 * @param OtherMesh World-space mesh tested against this cell's hull.
	 * @param EarlyExitDepth Caller's threshold for the typical `if (depth >= threshold)` rejection pattern.
	 *        See FNRawMeshUtils::GetIntersectDepth for the semantics of the short-circuit shortcuts.
	 * @return The deepest penetration distance between this cell's (world-space baked) hull and the supplied
	 *         world-space mesh, or -1.0 when their AABBs do not overlap or the meshes are otherwise unmeasurable
	 *         (see FNRawMeshUtils::GetIntersectDepth for the full set of sentinel cases).
	 * @note Intended for "max-allowed-penetration" threshold checks during graph expansion; use CheckHullIntersects
	 *       for an exact boolean overlap test.
	 */
	float GetHullIntersectDepth(const FNRawMesh& OtherMesh, const float EarlyExitDepth = MAX_flt) const
	{
		return FNRawMeshUtils::GetIntersectDepth(Hull, OtherMesh, EarlyExitDepth);
	}

	/**
	 * @return The raw input-data pointer used by the builder.
	 * @warning Only valid during graph construction; nulled by CleanupBuilderReferences afterward.
	 */
	FNVirtualCellData* GetInputDataPtr() const { return InputDataPtr; }

	/**
	 * Drop the builder-only input-data pointer once the graph is handed to the next stage.
	 * @note Forces the junction data to be built first. It is derived from InputDataPtr, so releasing that pointer
	 *       without realising the deferred fill would strand any later junction query on an empty map.
	 */
	void CleanupBuilderReferences()
	{
		EnsureJunctions();
		InputDataPtr = nullptr;
	}

	/** Flag whether this cell lies on the shortest-path hot path variant (spokes from start). */
	void SetHotPathShortest(bool bValue) { bHotPathShortest = bValue; }

	/** Flag whether this cell lies on the sequential hot path variant (nearest-first visiting chain). */
	void SetHotPathSequential(bool bValue) { bHotPathSequential = bValue; }

	/** @return true if this cell lies on the shortest-path hot path variant. */
	bool IsHotPathShortest() const { return bHotPathShortest; }

	/** @return true if this cell lies on the sequential hot path variant. */
	bool IsHotPathSequential() const { return bHotPathSequential; }

	/** @return true if this cell lies on either hot path variant. */
	bool IsHotPath() const { return bHotPathShortest || bHotPathSequential; }

	/** Populate LinkDetails with one entry per junction describing its connection state; no-op once generated. */
	void GenerateLinkDetails();

	/**
	 * @return The world-space AABB of the cell's baked hull — the broadphase key for hull-vs-mesh queries.
	 * @note This is the hull's own bounds, which is what GetIntersectDepth rejects against; it is not the same box
	 *       as the cell's authored WorldBounds. May be invalid when the hull has no vertices, in which case
	 *       GetIntersectDepth performs no AABB rejection and callers must not broadphase.
	 */
	const FBox& GetHullBounds() const { return Hull.Bounds; }

	/** @return Mutable access to the cell's world-space hull for intersection tests. */
	FNRawMesh& GetHull() { return Hull; }
	/** @return A by-value copy of the cell's hull for callers that need an independent mesh. */
	FNRawMesh GetHullCopy() { return Hull; }

private:
	/**
	 * Populates FreeJunctionKeys and WorldJunctions if they have not been built yet.
	 *
	 * Junction data is deferred because none of the placement tests read it — only a node that survives them and
	 * gets linked into the graph does. The builder constructs a candidate, runs the bounds and collision checks,
	 * and deletes it on rejection, so building the junction map up front meant every rejected candidate paid for a
	 * TMap fill plus a quaternion composition per junction and then threw it away.
	 *
	 * Safe to do lazily rather than under a lock: a candidate node is owned exclusively by the builder thread that
	 * created it for its entire life, and is never published to another thread. (Unlike the shared FNRawMesh caches,
	 * where the equivalent lazy fill was a genuine race.)
	 */
	void EnsureJunctions() const;

	/** Non-owning pointer to the input data this cell was chosen from; only valid during builder phase. */
	FNVirtualCellData* InputDataPtr;

	/** True once EnsureJunctions has run; see it for why this is deferred. */
	mutable bool bJunctionsBuilt = false;

	/** Junction keys that are still unlinked. Built on first access — read through EnsureJunctions, never directly. */
	mutable TArray<int32> FreeJunctionKeys;

	/** Map of junction key -> linked node (populated via Link/Unlink). */
	TMap<int32, FNAssemblyGraphNode*> Links;

	/**
	 * Junction key -> connector pairing identifier, for the subset of Links the connector pass created. Kept beside
	 * Links rather than inside it because every other consumer of Links only cares that a junction is linked at all,
	 * and an entry here is the only thing that distinguishes a routed pairing from a direct cell mating.
	 */
	TMap<int32, int32> ConnectorLinks;

	/** Cell junctions transformed into world space. Built on first access — read through EnsureJunctions. */
	mutable TMap<int32, FNCellJunctionDetails> WorldJunctions;

	/** Set just before spawning (game thread): does this cell lie on the shortest-path hot path (spokes from start). */
	bool bHotPathShortest = false;

	/** Set just before spawning (game thread): does this cell lie on the sequential hot path (nearest-first visiting chain). */
	bool bHotPathSequential = false;

	/** Generated just before spawning, before on main thread, traverses graph and generates useful additional data. */
	TArray<FNCellLinkDetails> LinkDetails;

	/** World-space axis-aligned bounds used for broad-phase intersection tests. */
	FBox WorldBounds;

	/** Cell hull baked into world space at construction (GetWorldPosition/Rotation already applied). */
	FNRawMesh Hull;

	/** Cell voxel data transformed into the graph's world space. */
	FNCellVoxelData WorldVoxel;

	/** Resolved template pointer; kept alive through the graph's lifetime. */
	TObjectPtr<UNCell> TemplatePtr;
};
