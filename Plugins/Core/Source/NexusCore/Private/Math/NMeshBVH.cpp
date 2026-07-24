// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NMeshBVH.h"

#include "Math/NTriangleUtils.h"
#include "Types/NRawMesh.h"
#include "Types/NRawMeshLoop.h"

FNMeshBVH::FNMeshBVH(const FNRawMesh& Mesh)
{
	// Collect baked triangles; the accelerated queries are only defined on a triangle mesh, so non-triangle loops
	// are dropped (mirroring the HasNonTris guard the brute-force callers apply before ever reaching this code).
	Triangles.Reserve(Mesh.Loops.Num());
	for (const FNRawMeshLoop& Loop : Mesh.Loops)
	{
		if (!Loop.IsTriangle())
		{
			continue;
		}

		FTriangle Tri;
		Tri.A = Mesh.Vertices[Loop.Indices[0]];
		Tri.B = Mesh.Vertices[Loop.Indices[1]];
		Tri.C = Mesh.Vertices[Loop.Indices[2]];
		Tri.Bounds = FBox(ForceInit);
		Tri.Bounds += Tri.A;
		Tri.Bounds += Tri.B;
		Tri.Bounds += Tri.C;
		Tri.Centroid = (Tri.A + Tri.B + Tri.C) / 3.0;
		Triangles.Add(Tri);
	}

	if (Triangles.Num() > 0)
	{
		// A median-split binary tree over N leaves has fewer than 2N nodes; reserve so the recursion never reallocates.
		Nodes.Reserve(Triangles.Num() * 2);
		BuildNode(0, Triangles.Num());
	}
}

int32 FNMeshBVH::BuildNode(const int32 Start, const int32 Count)
{
	const int32 NodeIndex = Nodes.AddDefaulted();

	// Enclose this range. Written by index (not reference) because the child recursion below appends to Nodes,
	// which can reallocate and invalidate any held FNode reference.
	FBox Bounds(ForceInit);
	for (int32 i = Start; i < Start + Count; ++i)
	{
		Bounds += Triangles[i].Bounds;
	}
	Nodes[NodeIndex].Bounds = Bounds;
	Nodes[NodeIndex].Start = Start;
	Nodes[NodeIndex].Count = Count;

	if (Count <= LeafThreshold)
	{
		return NodeIndex;
	}

	// Split along the axis of greatest centroid spread at the median centroid — cheap to build and gives balanced,
	// well-separated children for the AABB pruning the queries rely on.
	FBox CentroidBounds(ForceInit);
	for (int32 i = Start; i < Start + Count; ++i)
	{
		CentroidBounds += Triangles[i].Centroid;
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

	// Partition in a single O(Count) pass around the spatial midpoint of the centroids on the split axis — cheaper than
	// sorting the range, and balanced enough for the AABB pruning the queries rely on.
	const double SplitValue = CentroidBounds.Min[Axis] + CentroidExtent[Axis] * 0.5;
	int32 SplitIndex = Start;
	for (int32 i = Start; i < Start + Count; ++i)
	{
		if (Triangles[i].Centroid[Axis] < SplitValue)
		{
			Triangles.Swap(i, SplitIndex);
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

void FNMeshBVH::QueryNearest(const int32 NodeIndex, const FVector& Point, double& BestDistSq) const
{
	const FNode& Node = Nodes[NodeIndex];

	// Prune: nothing in this box can beat the running best.
	if (Node.Bounds.ComputeSquaredDistanceToPoint(Point) >= BestDistSq)
	{
		return;
	}

	if (Node.IsLeaf())
	{
		for (int32 i = Node.Start; i < Node.Start + Node.Count; ++i)
		{
			const FTriangle& Tri = Triangles[i];
			const double Dist = FNTriangleUtils::DistanceFromPointToTriangle(Point, Tri.A, Tri.B, Tri.C);
			const double DistSq = Dist * Dist;
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
			}
		}
		return;
	}

	// Descend the nearer child first so the farther one is more likely to prune.
	const int32 LeftChild = Node.Left;
	const int32 RightChild = Node.Right;
	const double LeftDistSq = Nodes[LeftChild].Bounds.ComputeSquaredDistanceToPoint(Point);
	const double RightDistSq = Nodes[RightChild].Bounds.ComputeSquaredDistanceToPoint(Point);
	if (LeftDistSq <= RightDistSq)
	{
		QueryNearest(LeftChild, Point, BestDistSq);
		QueryNearest(RightChild, Point, BestDistSq);
	}
	else
	{
		QueryNearest(RightChild, Point, BestDistSq);
		QueryNearest(LeftChild, Point, BestDistSq);
	}
}

void FNMeshBVH::QueryRayParity(const int32 NodeIndex, const FVector& Origin, const FVector& Direction,
	const FVector& InvDirection, int32& HitCount) const
{
	const FNode& Node = Nodes[NodeIndex];
	if (!RayIntersectsBox(Node.Bounds, Origin, InvDirection))
	{
		return;
	}

	if (Node.IsLeaf())
	{
		for (int32 i = Node.Start; i < Node.Start + Node.Count; ++i)
		{
			const FTriangle& Tri = Triangles[i];
			if (FNTriangleUtils::RayIntersectsTriangle(Origin, Direction, Tri.A, Tri.B, Tri.C))
			{
				++HitCount;
			}
		}
		return;
	}

	QueryRayParity(Node.Left, Origin, Direction, InvDirection, HitCount);
	QueryRayParity(Node.Right, Origin, Direction, InvDirection, HitCount);
}

bool FNMeshBVH::RayIntersectsBox(const FBox& Box, const FVector& Origin, const FVector& InvDirection)
{
	// Standard slab test clamped to the forward half-ray [0, +inf). Callers only ever pass the fixed non-axis-aligned
	// probe direction, whose reciprocal is finite on every axis, so the multiplies never produce an inf*0 NaN.
	double TMin = 0.0;
	double TMax = MAX_dbl;
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		double T1 = (Box.Min[Axis] - Origin[Axis]) * InvDirection[Axis];
		double T2 = (Box.Max[Axis] - Origin[Axis]) * InvDirection[Axis];
		if (T1 > T2)
		{
			Swap(T1, T2);
		}
		TMin = FMath::Max(TMin, T1);
		TMax = FMath::Min(TMax, T2);
		if (TMin > TMax)
		{
			return false;
		}
	}
	return true;
}

bool FNMeshBVH::IsPointInside(const FVector& Point) const
{
	if (Nodes.Num() == 0)
	{
		return false;
	}

	// Same probe as FNRawMeshUtils::IsRelativePointInside — deliberately irrational and non-axis-aligned so it never
	// grazes an axis-aligned hull edge, and so parity stays stable across an adjacent triangle pair. Its reciprocal is
	// finite on every axis, which is what lets RayIntersectsBox skip its zero-direction handling.
	static const FVector ProbeDirection(0.832050, 0.416025, 0.366092);
	const FVector InvProbeDirection(1.0 / ProbeDirection.X, 1.0 / ProbeDirection.Y, 1.0 / ProbeDirection.Z);

	int32 HitCount = 0;
	QueryRayParity(0, Point, ProbeDirection, InvProbeDirection, HitCount);
	return (HitCount & 1) == 1;
}

double FNMeshBVH::NearestSurfaceDistance(const FVector& Point) const
{
	if (Nodes.Num() == 0)
	{
		return 0.0;
	}

	double BestDistSq = MAX_dbl;
	QueryNearest(0, Point, BestDistSq);
	return FMath::Sqrt(BestDistSq);
}

float FNMeshBVH::GetPointDepth(const FVector& Point) const
{
	if (Nodes.Num() == 0)
	{
		return -1.0f;
	}

	// AABB reject first, matching the early-out in FNRawMeshUtils::GetIntersectDepth(mesh, point).
	if (!Nodes[0].Bounds.IsInsideOrOn(Point))
	{
		return -1.0f;
	}

	if (!IsPointInside(Point))
	{
		return -1.0f;
	}

	return static_cast<float>(NearestSurfaceDistance(Point));
}
