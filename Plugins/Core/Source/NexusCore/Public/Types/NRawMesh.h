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
	 * Lazily populates per-triangle face-plane data (raw Cross normal, plane offset, inverse normal length)
	 * derived from Loops + Vertices. Called automatically by the convex-path consumers in FNRawMeshUtils
	 * the first time they need it after a topology / vertex change. Safe to call repeatedly — no-ops when
	 * the cache is already valid.
	 * @note Only triangle loops contribute meaningful entries. Non-triangle loops (or degenerate triangles
	 *       with a zero-length cross product) yield a zero normal and zero InvNormalLen, which the
	 *       consumers treat as "skip this face".
	 */
	void EnsureCachedFacePlanes() const;

	/** @return true when the face-plane cache reflects the current Loops / Vertices state. */
	bool HasCachedFacePlanes() const { return bCachedFacePlanesValid; }

	/** @return Per-triangle raw face normals (Cross(V1-V0, V2-V0), un-normalized). Lazily populated. */
	const TArray<FVector>& GetCachedFaceNormals() const { EnsureCachedFacePlanes(); return CachedFaceNormals; }

	/** @return Per-triangle plane offsets such that PlaneD[i] = Dot(Normal[i], V0[i]). Lazily populated. */
	const TArray<double>& GetCachedFacePlaneD() const { EnsureCachedFacePlanes(); return CachedFacePlaneD; }

	/**
	 * @return Per-triangle 1/||Normal||. Zero for degenerate triangles — consumers should treat zero
	 *         entries as "no perpendicular distance contribution".
	 */
	const TArray<double>& GetCachedFaceInvNormalLen() const { EnsureCachedFacePlanes(); return CachedFaceInvNormalLen; }

	/**
	 * Marks the face-plane cache stale so the next consumer query rebuilds it. Called by every mutator
	 * inside FNRawMesh; external callers that bypass those mutators (the friend classes) must invoke
	 * this themselves after touching Vertices or Loops.
	 */
	void InvalidateCachedFacePlanes() const { bCachedFacePlanesValid = false; }

	/**
	 * Cached convexity result. Lazily re-evaluated on first read after a mutator marks validation dirty,
	 * so consumers never see stale data even when many edits happen between queries.
	 */
	bool IsConvex() const { EnsureValidated(); return bIsConvex; }

	/**
	 * Cached AABB-set flag. Lazily re-evaluated on first read after a mutator marks validation dirty.
	 */
	bool HasBounds() const { EnsureValidated(); return bHasBounds; }

	/**
	 * Cached non-triangle-loop flag. Lazily re-evaluated on first read after a mutator marks validation dirty.
	 */
	bool HasNonTris() const { EnsureValidated(); return bHasNonTris; }

	/**
	 * Marks the cached convexity / non-tri / bounds flags stale so the next IsConvex / HasNonTris /
	 * HasBounds query (or the next explicit Validate call) re-evaluates them. Cheap — just sets a
	 * bit — so mutators can call it freely without paying the O(V * F) CheckConvex cost up front.
	 */
	void InvalidateValidation() const { bValidationDirty = true; }

	/**
	 * Runs the convexity / non-tri / bounds checks if the validation cache is dirty, otherwise no-ops.
	 * Triggered automatically by IsConvex / HasNonTris / HasBounds; consumers that want eager
	 * evaluation can call this directly. Honors bIsChaosGenerated as a trust signal that skips
	 * the work entirely.
	 */
	void EnsureValidated() const;

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
		InvalidateCachedFacePlanes();
		InvalidateValidation();
	}
	
	/**
	 * Bakes a full transform (translation, rotation, scale) into every vertex and the center, then
	 * refreshes Bounds from the transformed vertices. No-op when Transform is the identity.
	 * @param Transform Transform to apply to the mesh, in vertex space.
	 */
	void ApplyTransform(const FTransform& Transform)
	{
		if (Transform.Equals(FTransform::Identity)) return;

		FBox NewBounds(ForceInit);
		const int32 Count = Vertices.Num();
		FVector CenterCalc;
		
		for (int32 i = 0; i < Count; i++)
		{
			Vertices[i] = Transform.TransformPosition(Vertices[i]);
			NewBounds += Vertices[i];
			CenterCalc += Vertices[i];
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
		
		Bounds = NewBounds;
		bHasBounds = (Count > 0);
		InvalidateCachedFacePlanes();
		InvalidateValidation();
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
		InvalidateCachedFacePlanes();
		InvalidateValidation();
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
	 * Forces an immediate re-evaluation of the convex / non-triangle / bounds flags. Equivalent to
	 * marking the cache dirty and calling EnsureValidated(); kept for back-compat with callers that
	 * want eager evaluation (typically factory methods finishing mesh construction).
	 * @note Skipped for meshes marked as Chaos-generated, since those are trusted to already be tri-based and valid.
	 */
	void Validate() const
	{
		bValidationDirty = true;
		EnsureValidated();
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

	/** Cached result of the most recent convexity check. Mutable so EnsureValidated() can refresh it lazily from const queries. */
	UPROPERTY(VisibleAnywhere)
	mutable bool bIsConvex = false;

	/** When true, the mesh originated from a Chaos destruction result and should be trusted as-is. */
	UPROPERTY(VisibleAnywhere)
	bool bIsChaosGenerated = false;

	/** Cached result of the most recent non-triangle loop check. Mutable for the same lazy-refresh reason as bIsConvex. */
	UPROPERTY(VisibleAnywhere)
	mutable bool bHasNonTris = false;

	/** Cached result of the most bounds check. Mutable for the same lazy-refresh reason as bIsConvex. */
	UPROPERTY(VisibleAnywhere)
	mutable bool bHasBounds = false;

	/**
	 * When true, the cached validation flags (bIsConvex, bHasNonTris, bHasBounds) are stale and will be
	 * recomputed by the next IsConvex / HasNonTris / HasBounds query (or by an explicit Validate call).
	 * Set by every internal mutator; external code that bypasses those mutators must call
	 * InvalidateValidation() after touching Loops or Vertices.
	 */
	mutable bool bValidationDirty = false;
	
	UPROPERTY(VisibleAnywhere)
	mutable bool bHasAppliedTransform = false;

	/**
	 * Transient face-plane cache derived from Loops + Vertices. Not serialized; rebuilt on demand
	 * after any topology / vertex change. Marked mutable so EnsureCachedFacePlanes can populate it
	 * from const consumers (the convex-path queries in FNRawMeshUtils).
	 */
	mutable bool bCachedFacePlanesValid = false;
	mutable TArray<FVector> CachedFaceNormals;
	mutable TArray<double> CachedFacePlaneD;
	mutable TArray<double> CachedFaceInvNormalLen;
};