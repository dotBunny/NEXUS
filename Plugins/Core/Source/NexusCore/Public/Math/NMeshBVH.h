// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

struct FNRawMesh;

/**
 * Immutable bounding-volume hierarchy over the triangles of an FNRawMesh, built once and queried many times.
 *
 * Accelerates the point-containment (odd-parity ray cast) and nearest-surface-distance queries that
 * FNRawMeshUtils::ComputePointDepthInside performs by brute force, turning each query from O(triangles) into
 * roughly O(log triangles) over the geometry the query actually touches. Intended for author-time diagnostics
 * that repeatedly sample a large, static, non-convex mesh at a handful of points per frame — most notably World
 * Assembly's merged world-collision mesh sampled by the bone visualizer.
 *
 * The source mesh's vertices are assumed to already be in the query space (world space for the merged collision
 * mesh), matching the "vertices are baked" convention used throughout FNRawMeshUtils. Non-triangle loops are
 * skipped at build time, so the accelerated queries are only equivalent to the brute-force sweep on a triangle
 * mesh (which is the domain those queries are defined on anyway).
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/math/mesh-bvh/">FNMeshBVH</a>
 */
class NEXUSCORE_API FNMeshBVH
{
public:
	FNMeshBVH() = default;

	/**
	 * Builds a BVH over Mesh's triangle loops.
	 * @param Mesh Source mesh; its triangle loops are read in the mesh's (baked) space. Non-triangle loops are skipped.
	 * @note IsEmpty() is true when Mesh contributes no triangles; every query then returns its empty-mesh answer.
	 */
	explicit FNMeshBVH(const FNRawMesh& Mesh);

	/** @return true when the hierarchy holds no triangles. */
	bool IsEmpty() const { return Triangles.Num() == 0; }

	/** @return The world-space AABB enclosing every triangle, or an invalid box when empty. */
	FBox GetBounds() const { return Nodes.Num() > 0 ? Nodes[0].Bounds : FBox(ForceInit); }

	/**
	 * Point-in-mesh test by odd-parity ray cast, accelerated by the hierarchy.
	 *
	 * Bit-identical in result to FNRawMeshUtils::IsRelativePointInside on the same (triangle, closed-manifold)
	 * mesh: the traversal visits every triangle whose AABB the probe ray crosses — a superset of the triangles the
	 * ray can pierce — and each triangle lives in exactly one leaf, so the parity count matches the brute-force
	 * sweep exactly. The probe direction matches IsRelativePointInside so grazed shared edges are missed by both
	 * adjacent triangles, keeping the count stable.
	 * @param Point Query point in the mesh's (baked) space.
	 * @return true when Point is enclosed by the surface; false when outside or the hierarchy is empty.
	 */
	bool IsPointInside(const FVector& Point) const;

	/**
	 * Minimum Euclidean distance from Point to any triangle surface, accelerated by AABB-distance pruning.
	 * Equal to the brute-force minimum — pruning only discards subtrees that provably cannot hold a closer triangle.
	 * @param Point Query point in the mesh's (baked) space.
	 * @return The nearest surface distance, or 0 when the hierarchy is empty.
	 */
	double NearestSurfaceDistance(const FVector& Point) const;

	/**
	 * Penetration depth of Point into the mesh — the accelerated drop-in for the per-corner
	 * GetIntersectDepth(mesh, point) call in the bone visualizer.
	 *
	 * Matches FNRawMeshUtils::ComputePointDepthInside for a non-convex closed manifold: the distance to the
	 * nearest surface when Point is inside, or -1 when it is outside the mesh's AABB, outside the surface, or the
	 * hierarchy is empty.
	 * @param Point Query point in the mesh's (baked) space.
	 */
	float GetPointDepth(const FVector& Point) const;

private:
	/** A single baked triangle plus the precomputed data the build and queries need. */
	struct FTriangle
	{
		FVector A = FVector::ZeroVector;
		FVector B = FVector::ZeroVector;
		FVector C = FVector::ZeroVector;
		FBox Bounds = FBox(ForceInit);
		FVector Centroid = FVector::ZeroVector;
	};

	/** A hierarchy node: a leaf owns the contiguous triangle range [Start, Start + Count); interior nodes own children. */
	struct FNode
	{
		FBox Bounds = FBox(ForceInit);
		int32 Start = 0;
		int32 Count = 0;
		int32 Left = INDEX_NONE;
		int32 Right = INDEX_NONE;

		bool IsLeaf() const { return Left == INDEX_NONE; }
	};

	/** Recursively builds a node over Triangles[Start, Start + Count), reordering that range, and returns its node index. */
	int32 BuildNode(int32 Start, int32 Count);

	/** Descends NodeIndex, tightening BestDistSq (squared) toward the nearest triangle, pruning subtrees that can't beat it. */
	void QueryNearest(int32 NodeIndex, const FVector& Point, double& BestDistSq) const;

	/** Descends NodeIndex, counting strict-interior triangle crossings of the ray (Origin, Direction) into HitCount. */
	void QueryRayParity(int32 NodeIndex, const FVector& Origin, const FVector& Direction, const FVector& InvDirection, int32& HitCount) const;

	/** Slab test for the ray (Origin, 1/Direction) against Box over t in [0, +inf). InvDirection must be finite. */
	static bool RayIntersectsBox(const FBox& Box, const FVector& Origin, const FVector& InvDirection);

	/** Leaves hold at most this many triangles; larger nodes split. */
	static constexpr int32 LeafThreshold = 4;

	/** Baked triangles, reordered during build so each leaf references a contiguous range. */
	TArray<FTriangle> Triangles;

	/** Flattened node pool; Nodes[0] is the root when non-empty. */
	TArray<FNode> Nodes;
};
