// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NBoundsBVH.h"

FNBoundsBVH::FNBoundsBVH(const TConstArrayView<FBox> Bounds)
{
	// Invalid entries are dropped rather than carried: an invalid FBox can never satisfy Intersect, so keeping one
	// would only widen a node's enclosing box and cost the traversal pruning power. The surviving entries record
	// their original index, so dropping them never shifts what a query reports.
	Entries.Reserve(Bounds.Num());
	for (int32 i = 0; i < Bounds.Num(); ++i)
	{
		if (!Bounds[i].IsValid)
		{
			continue;
		}

		FEntry Entry;
		Entry.Bounds = Bounds[i];
		Entry.Centroid = Bounds[i].GetCenter();
		Entry.SourceIndex = i;
		Entries.Add(Entry);
	}

	if (Entries.Num() > 0)
	{
		// A median-split binary tree over N leaves has fewer than 2N nodes; reserve so the recursion never reallocates.
		Nodes.Reserve(Entries.Num() * 2);
		BuildNode(0, Entries.Num());
	}
}

int32 FNBoundsBVH::BuildNode(const int32 Start, const int32 Count)
{
	const int32 NodeIndex = Nodes.AddDefaulted();

	// Enclose this range. Written by index (not reference) because the child recursion below appends to Nodes,
	// which can reallocate and invalidate any held FNode reference.
	FBox Bounds(ForceInit);
	for (int32 i = Start; i < Start + Count; ++i)
	{
		Bounds += Entries[i].Bounds;
	}
	Nodes[NodeIndex].Bounds = Bounds;
	Nodes[NodeIndex].Start = Start;
	Nodes[NodeIndex].Count = Count;

	if (Count <= LeafThreshold)
	{
		return NodeIndex;
	}

	// Split along the axis of greatest centroid spread at the spatial midpoint — cheap to build and gives balanced,
	// well-separated children for the AABB pruning the query relies on. Same strategy as FNMeshBVH.
	FBox CentroidBounds(ForceInit);
	for (int32 i = Start; i < Start + Count; ++i)
	{
		CentroidBounds += Entries[i].Centroid;
	}
	const FVector CentroidExtent = CentroidBounds.GetSize();
	int32 Axis = 0;
	if (CentroidExtent.Y > CentroidExtent.X)
	{
		Axis = 1;
	}
	if (CentroidExtent.Z > CentroidExtent[Axis])
	{
		Axis = 2;
	}

	if (CentroidExtent[Axis] <= UE_KINDA_SMALL_NUMBER)
	{
		// Every centroid coincides — no split separates them, so keep this as a (larger) leaf to avoid infinite recursion.
		return NodeIndex;
	}

	// Partition in a single O(Count) pass around the spatial midpoint of the centroids on the split axis — cheaper
	// than sorting the range, and balanced enough for the pruning the query relies on.
	const double SplitValue = CentroidBounds.Min[Axis] + CentroidExtent[Axis] * 0.5;
	int32 SplitIndex = Start;
	for (int32 i = Start; i < Start + Count; ++i)
	{
		if (Entries[i].Centroid[Axis] < SplitValue)
		{
			Entries.Swap(i, SplitIndex);
			++SplitIndex;
		}
	}

	// A degenerate split (every centroid landed on one side) can't recurse; fall back to a median count split.
	if (SplitIndex == Start || SplitIndex == Start + Count)
	{
		SplitIndex = Start + Count / 2;
	}

	const int32 LeftIndex = BuildNode(Start, SplitIndex - Start);
	const int32 RightIndex = BuildNode(SplitIndex, Start + Count - SplitIndex);
	Nodes[NodeIndex].Left = LeftIndex;
	Nodes[NodeIndex].Right = RightIndex;
	return NodeIndex;
}

// QueryOverlaps and QueryNode are templated on the result array's allocator so callers in a hot loop can pass a
// TInlineAllocator array and stay off the heap; both live in the header.
