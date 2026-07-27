// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * Immutable bounding-volume hierarchy over an indexed set of axis-aligned boxes, built once and queried many times.
 *
 * The object-level counterpart to FNMeshBVH: where that accelerates point queries against the triangles *within*
 * one mesh, this answers "which of these N objects could overlap this box" without touching the other N-1. It is
 * the broadphase for callers that currently sweep a whole collection per query — most notably World Assembly's
 * graph builder, which tests every candidate cell placement against every world-collision mesh and every
 * previously-placed cell.
 *
 * Bounds are consumed in whatever space the caller supplies; the hierarchy never transforms them, so query boxes
 * must be expressed in that same space. Entries whose bounds are invalid are dropped at build time — they can
 * never overlap anything — but every surviving entry keeps its original index, so results index straight back into
 * the caller's array.
 *
 * Immutable after construction, so a built tree can be shared across threads without synchronisation. Rebuild it
 * when the underlying set changes; there is no incremental insert.
 */
class NEXUSCORE_API FNBoundsBVH
{
public:
	FNBoundsBVH() = default;

	/**
	 * Builds a hierarchy over Bounds.
	 * @param Bounds Source bounds, indexed by position. Invalid entries are skipped; the rest keep their index.
	 * @note IsEmpty() is true when no entry has valid bounds; every query then returns no overlaps.
	 */
	explicit FNBoundsBVH(TConstArrayView<FBox> Bounds);

	/** @return true when the hierarchy holds no entries. */
	bool IsEmpty() const { return Entries.Num() == 0; }

	/** @return The number of indexed entries, which excludes any source entry whose bounds were invalid. */
	int32 Num() const { return Entries.Num(); }

	/** @return The AABB enclosing every indexed entry, or an invalid box when empty. */
	FBox GetBounds() const { return Nodes.Num() > 0 ? Nodes[0].Bounds : FBox(ForceInit); }

	/**
	 * Collects the index of every entry whose bounds overlap QueryBox.
	 *
	 * Equivalent to sweeping the source array and keeping each index where FBox::Intersect is true — the traversal
	 * only ever prunes subtrees whose enclosing box already fails that test.
	 * @param QueryBox Box to test against, in the same space as the source bounds. An invalid box matches nothing.
	 * @param OutIndices Receives the matching source indices. **Reset, not reallocated** — pass the same array
	 *        across queries and it settles at zero allocations per call after the first few. Templated on the
	 *        allocator so a caller in a hot loop can pass a TInlineAllocator array and stay off the heap entirely.
	 * @note Results arrive in traversal order, which is deterministic for a given tree but is not ascending index
	 *       order. Callers that need a stable order must sort.
	 */
	template <typename AllocatorType>
	void QueryOverlaps(const FBox& QueryBox, TArray<int32, AllocatorType>& OutIndices) const
	{
		// Reset rather than Empty so a caller reusing one array across queries keeps its allocation.
		OutIndices.Reset();

		if (Nodes.Num() == 0 || !QueryBox.IsValid)
		{
			return;
		}

		QueryNode(0, QueryBox, OutIndices);
	}

private:
	/** One indexed source box, carrying the index it had in the array the hierarchy was built from. */
	struct FEntry
	{
		FBox Bounds = FBox(ForceInit);
		FVector Centroid = FVector::ZeroVector;
		int32 SourceIndex = INDEX_NONE;
	};

	/** A hierarchy node: a leaf owns the contiguous entry range [Start, Start + Count); interior nodes own children. */
	struct FNode
	{
		FBox Bounds = FBox(ForceInit);
		int32 Start = 0;
		int32 Count = 0;
		int32 Left = INDEX_NONE;
		int32 Right = INDEX_NONE;

		bool IsLeaf() const { return Left == INDEX_NONE; }
	};

	/** Recursively builds a node over Entries[Start, Start + Count), reordering that range, and returns its node index. */
	int32 BuildNode(int32 Start, int32 Count);

	/** Descends NodeIndex, appending the source index of every entry whose bounds overlap QueryBox. */
	template <typename AllocatorType>
	void QueryNode(const int32 NodeIndex, const FBox& QueryBox, TArray<int32, AllocatorType>& OutIndices) const
	{
		const FNode& Node = Nodes[NodeIndex];

		// Prune: nothing enclosed by this box can overlap when the box itself does not.
		if (!Node.Bounds.Intersect(QueryBox))
		{
			return;
		}

		if (Node.IsLeaf())
		{
			for (int32 i = Node.Start; i < Node.Start + Node.Count; ++i)
			{
				if (Entries[i].Bounds.Intersect(QueryBox))
				{
					OutIndices.Add(Entries[i].SourceIndex);
				}
			}
			return;
		}

		QueryNode(Node.Left, QueryBox, OutIndices);
		QueryNode(Node.Right, QueryBox, OutIndices);
	}

	/** Leaves hold at most this many entries; larger nodes split. Matches FNMeshBVH's leaf sizing. */
	static constexpr int32 LeafThreshold = 4;

	/** Indexed entries, reordered during build so each leaf references a contiguous range. */
	TArray<FEntry> Entries;

	/** Flattened node pool; Nodes[0] is the root when non-empty. */
	TArray<FNode> Nodes;
};
