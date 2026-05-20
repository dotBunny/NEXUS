// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/BaseDynamicMeshComponent.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMeshLoop.h"
#include "NRawMesh.generated.h"


/**
 * Lightweight CPU-side mesh representation used by World Assembly and blockout tools.
 *
 * Stores a shared vertex buffer plus one or more ordered loops that reference into it. Supports
 * convexity / non-tri validation, rigid-body rotation around a pivot, and conversion into
 * Unreal Engine's FDynamicMesh3 for richer geometry work.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRawMesh
{
	friend class FNWorldAssemblyUtils;
	friend class FNCellRootComponentVisualizer;
	friend class FNRawMeshUtils;
	friend class FNRawMeshFactory;

	GENERATED_BODY()

	/** The vertices of the mesh. */
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Vertices;

	/** The relative center of the mesh. */
	UPROPERTY(VisibleAnywhere)
	FVector Center = FVector::ZeroVector;
	
	/** Relative AABB **/
	UPROPERTY(VisibleAnywhere)
	FBox Bounds;

	/**
	 * Ordered shape-edge definition.
	 * @note The indices are ordered, so 1/2/3 for a triangle. This is NOT looped; therefore, you must close the loop if line drawing.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FNRawMeshLoop> Loops;

	/**
	 * Pre-triangulation n-gonal face definition, in winding order. Optional — when populated, CheckConvex
	 * walks these polygonal faces instead of Loops, which avoids false-negative convexity rejections caused
	 * by the per-triangle plane test on fan-triangulated coplanar faces.
	 * @note Indices reference the same Vertices buffer as Loops. Index-stable vertex mutations (move/scale)
	 * preserve FaceLoops; topology changes (add/remove vertex, re-triangulate from scratch) do not.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FNRawMeshLoop> FaceLoops;

	/**
	 * Flattens Loops into a single contiguous index buffer.
	 * @return A concatenation of every loop's indices.
	 */
	TArray<int32> GetFlatIndices() const;

	/**
	 * Extracts every unique undirected edge across all Loops as a (min, max) vertex-index pair. Loops
	 * are treated as closed polygons (last vertex wraps back to first), and shared edges between adjacent
	 * faces collapse to a single entry.
	 * @return Deduplicated list of edges as vertex-index pairs into Vertices.
	 */
	TArray<FIntVector2> GetEdgeIndices() const;

	/** Re-triangulates non-triangular loops in place so every loop is a 3-vertex triangle. */
	void ConvertToTriangles();

	/**
	 * Rebuilds FaceLoops from a triangulated Loops by grouping coplanar-adjacent triangles back into n-gonal faces.
	 * The inverse of ConvertToTriangles for known-polygonal sources, and the only way to get a polygonal face
	 * description for triangle-only inputs (static-mesh imports, physics tri meshes).
	 *
	 * Algorithm: per-triangle Newell normals, edge → triangle adjacency, union-find merging triangles that share
	 * an edge AND have near-parallel normals AND are mutually coplanar within tolerance, then a boundary-edge walk
	 * to recover each group's polygon in winding order. Singleton groups remain as triangles.
	 *
	 * @param AngleToleranceDeg Maximum normal-angle difference for triangles to be considered the same face. Default
	 *        1 degree picks up tessellated planar faces without merging shallow dihedrals.
	 * @param RelativeDistanceTolerance Coplanarity threshold expressed as a fraction of mesh extent. Default 1e-4
	 *        absorbs FP drift on coordinates at ~1000-unit scale (~0.1-unit absolute tolerance).
	 * @note No-op when Loops is empty or contains any non-triangle. Existing FaceLoops content is discarded.
	 *       Faces with internal holes are emitted as their outer boundary only (acceptable for convexity testing —
	 *       convex hull faces don't have holes anyway).
	 */
	void CalculateFaceLoops(double AngleToleranceDeg = 1.0, double RelativeDistanceTolerance = 1e-4);

	/**
	 * Cached convexity result from the most recent Validate(). Defaults to false; returns false until
	 * Validate() (or an external setter via a friend) has populated it. Not live — mutating Vertices
	 * or Loops can leave this stale.
	 */
	bool IsConvex() const { return bIsConvex; }

	/**
	 * Cached AABB-set flag from the most recent Validate(). Defaults to false; CalculateCenterAndBounds()
	 * and RotatedAroundPivot() also keep it in sync. Not live — direct vertex mutation can leave this stale.
	 */
	bool HasBounds() const { return bHasBounds; }

	/**
	 * Cached non-triangle-loop flag from the most recent Validate(). Defaults to false; returns false until
	 * Validate() has populated it. Not live — mutating Loops can leave this stale.
	 */
	bool HasNonTris() const { return bHasNonTris; }

	/**
	 * Deep equality comparison: vertices, loops, center, bounds, and the cached validation flags
	 * (bIsConvex, bHasNonTris, bHasBounds, bIsChaosGenerated). Two meshes with identical geometry
	 * but different cached flags will compare not-equal — call Validate() first if that matters.
	 */
	bool operator==(const FNRawMesh& Other) const
	{
		return bIsConvex == Other.bIsConvex
			&& bHasNonTris == Other.bHasNonTris
			&& bHasBounds == Other.bHasBounds
			&& bIsChaosGenerated == Other.bIsChaosGenerated
			&& Center == Other.Center
			&& Bounds == Other.Bounds
			&& Vertices == Other.Vertices
			&& Loops == Other.Loops
			&& FaceLoops == Other.FaceLoops;
	}

	bool operator!=(const FNRawMesh& Other) const
	{
		return !(*this == Other);
	}

	/**
	 * Rotates every vertex and the center around a world-space pivot, and refreshes Bounds from the
	 * rotated vertices.
	 * @param WorldPoint Pivot around which the rotation occurs.
	 * @param Rotation Rotation to apply.
	 */
	void RotatedAroundPivot(const FVector& WorldPoint, const FRotator& Rotation)
	{
		const FQuat Quat = Rotation.Quaternion();
		FBox NewBounds(ForceInit);
		const int32 Count = Vertices.Num();
		for (int32 i = 0; i < Count; i++)
		{
			Vertices[i] = Quat.RotateVector(Vertices[i] - WorldPoint) + WorldPoint;
			NewBounds += Vertices[i];
		}
		Center = Quat.RotateVector(Center - WorldPoint) + WorldPoint;
		Bounds = NewBounds;
		bHasBounds = (Count > 0);
	}

	/**
	 * Bakes Scale into every vertex, then refreshes Center and Bounds. No-op when Scale is the identity.
	 * @param Scale Per-axis scale factors. Callers with an FTransform should pass Transform.GetScale3D().
	 */
	void ApplyScale(const FVector& Scale)
	{
		if (Scale.Equals(FVector::OneVector)) return;

		const int32 Count = Vertices.Num();
		for (int32 i = 0; i < Count; i++)
		{
			Vertices[i] *= Scale;
		}
		CalculateCenterAndBounds();
	}

	/**
	 * Recomputes Center as the mean of Vertices and Bounds as the AABB enclosing them.
	 */
	void CalculateCenterAndBounds()
	{
		FVector CenterCalc = FVector::ZeroVector;
		FBox BoundingBox(ForceInit);
		for (int32 i = 0; i < Vertices.Num(); i++)
		{
			CenterCalc += Vertices[i];
			BoundingBox += Vertices[i];
		}

		// Ensure that we never divide by zero
		if (Vertices.Num() > 0)
		{
			Center = CenterCalc / Vertices.Num();
		}
		else
		{
			Center = FVector::ZeroVector;
		}

		Bounds = BoundingBox;
		bHasBounds = Bounds.IsValid != 0;
	}

	/**
	 * Re-evaluates convex / non-triangle flags from the current vertex and loop data.
	 * @note Skipped for meshes marked as Chaos-generated, since those are trusted to already be tri-based and valid.
	 */
	void Validate()
	{
		if (bIsChaosGenerated)
		{
			return;
		}
		bIsConvex = CheckConvex();
		bHasNonTris = CheckNonTris();
		bHasBounds = CheckBounds();
	}

	/**
	 * Creates an FDynamicMesh3 copy suitable for use with UE's dynamic mesh processing APIs.
	 * @param bProcessMesh When true, runs additional cleanup on the resulting dynamic mesh.
	 */
	FDynamicMesh3 CreateDynamicMesh(bool bProcessMesh = false) const;

	/**
	 * Inserts a new vertex at the midpoint of the edge between VertexAIndex and VertexBIndex and
	 * splices it into every Loop and FaceLoop that owns that edge, so the affected face(s) become
	 * (..., A, M, B, ...). Both sides of a shared edge are updated together, which is what keeps
	 * the result T-junction free on a closed mesh.
	 *
	 * Loops: a triangle that owned the edge becomes a quad after the insert and is immediately
	 * fan-triangulated from M — producing the natural geometric split — so Loops stays
	 * triangulated. Non-triangulated loops are spliced but not re-triangulated; call
	 * ConvertToTriangles() if the caller relies on that invariant.
	 *
	 * FaceLoops: M is spliced into any face that owned the edge; no re-triangulation needed.
	 *
	 * Bookkeeping: clears bIsChaosGenerated (the chaos provenance no longer applies after a
	 * topology change), then runs CalculateCenterAndBounds() and Validate() so the cached
	 * convexity / bounds / non-triangle flags reflect the new state.
	 *
	 * @param VertexAIndex First endpoint of the edge to split.
	 * @param VertexBIndex Second endpoint of the edge to split.
	 * @return Index of the newly inserted midpoint vertex, or INDEX_NONE when an endpoint index
	 *         is invalid, the two indices are equal, or no loop/face references the edge.
	 * @note Edge direction is order-agnostic — A→B and B→A both match.
	 */
	int32 SplitEdge(int32 VertexAIndex, int32 VertexBIndex);

private:
	/**
	 * @return true if Vertices form a convex hull; false otherwise. Recomputed by Validate.
	 * @note Walks FaceLoops when non-empty (the polygonal face description); falls back to Loops otherwise.
	 * Using FaceLoops avoids false negatives on fan-triangulated meshes where coplanar neighbor triangles
	 * drift slightly off each other's plane after any vertex edit.
	 */
	bool CheckConvex() const;
	/** @return true if any loop in the mesh has more than three vertices. Recomputed by Validate. */
	bool CheckNonTris() const;
	/** @return true when Bounds has been populated (non-default FBox). Recomputed by Validate. */
	FORCEINLINE bool CheckBounds() const { return Bounds.IsValid != 0; };

	/**
	 * Newell's-method face normal for an arbitrary (possibly slightly non-planar) polygon loop.
	 * @param Indices Loop indices into VertexBuffer, in winding order.
	 * @param VertexBuffer Vertex pool the indices reference.
	 * @return Unit-length face normal, or zero for degenerate / collinear loops.
	 */
	FORCEINLINE static FVector ComputeLoopNormal(const TArray<int32>& Indices, const TArray<FVector>& VertexBuffer)
	{
		const int32 Count = Indices.Num();
		FVector Normal = FVector::ZeroVector;
		for (int32 k = 0; k < Count; k++)
		{
			const FVector& A = VertexBuffer[Indices[k]];
			const FVector& B = VertexBuffer[Indices[(k + 1) % Count]];
			Normal.X += (A.Y - B.Y) * (A.Z + B.Z);
			Normal.Y += (A.Z - B.Z) * (A.X + B.X);
			Normal.Z += (A.X - B.X) * (A.Y + B.Y);
		}
		return Normal.GetSafeNormal();
	}

	/** Cached result of the most recent convexity check. */
	UPROPERTY(VisibleAnywhere)
	bool bIsConvex = false;

	/** When true, the mesh originated from a Chaos destruction result and should be trusted as-is. */
	UPROPERTY(VisibleAnywhere)
	bool bIsChaosGenerated = false;

	/** Cached result of the most recent non-triangle loop check. */
	UPROPERTY(VisibleAnywhere)
	bool bHasNonTris = false;
	
	/** Cached result of the most bounds check. */
	UPROPERTY(VisibleAnywhere)
	bool bHasBounds = false;
};