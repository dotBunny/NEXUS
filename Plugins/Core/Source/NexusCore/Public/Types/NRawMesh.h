// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/BaseDynamicMeshComponent.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMeshLoop.h"
#include "NRawMesh.generated.h"


/**
 * Lightweight CPU-side mesh representation used by ProcGen and blockout tools.
 *
 * Stores a shared vertex buffer plus one or more ordered loops that reference into it. Supports
 * convexity / non-tri validation, rigid-body rotation around a pivot, and conversion into
 * Unreal Engine's FDynamicMesh3 for richer geometry work.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNRawMesh
{
	friend class FNProcGenUtils;
	friend class FNCellRootComponentVisualizer;
	friend class FNRawMeshUtils;

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
	 * Flattens Loops into a single contiguous index buffer.
	 * @return A concatenation of every loop's indices.
	 */
	TArray<int32> GetFlatIndices();

	/** Re-triangulates non-triangular loops in place so every loop is a 3-vertex triangle. */
	void ConvertToTriangles();

	/** Whether the mesh has been validated as convex. */
	bool IsConvex() const { return bIsConvex; }

	/** Whether the mesh has an AABB set. */
	bool HasBounds() const { return bHasBounds; }
	
	/** Whether any loop in the mesh has more than three vertices. */
	bool HasNonTris() const { return bHasNonTris; }

	/**
	 * Deep equality comparison (vertices, loops, validation flags and center).
	 * @param Other Right-hand side of the comparison.
	 */
	bool IsEqual(const FNRawMesh& Other) const
	{
		if (bIsConvex != Other.bIsConvex || bHasNonTris != Other.bHasNonTris || 
			bHasBounds != Other.bHasBounds || bIsChaosGenerated != Other.bIsChaosGenerated)
		{
			return false;
		}
		if (Center != Other.Center) return false;
		if (Bounds != Other.Bounds) return false;
		if (Loops.Num() != Other.Loops.Num()) return false;
		if (Vertices != Other.Vertices) return false;
		for (int32 i = 0; i < Loops.Num(); i++)
		{
			if (!Loops[i].IsEqual(Other.Loops[i])) return false;
		}
		return true;
	}

	/**
	 * Rotates every vertex and the center around a world-space pivot.
	 * @param WorldPoint Pivot around which the rotation occurs.
	 * @param Rotation Rotation to apply.
	 */
	void RotatedAroundPivot(const FVector& WorldPoint, const FRotator& Rotation)
	{
		const int Count = Vertices.Num();
		for (int i = 0; i < Count; i++)
		{
			Vertices[i] = FNVectorUtils::RotatedAroundPivot(Vertices[i] + WorldPoint, WorldPoint, Rotation);
		}
		Center = FNVectorUtils::RotatedAroundPivot(Center + WorldPoint, WorldPoint, Rotation);
	}

	/**
	 * Recomputes Center as the mean of Vertices and Bounds as the AABB enclosing them.
	 */
	void CalculateCenterAndBounds()
	{
		FVector CenterCalc;
		FBox BoundingBox(ForceInit);
		for (int i = 0; i < Vertices.Num(); i++)
		{
			CenterCalc += Vertices[i];
			BoundingBox += Vertices[i];
		}
		Center = CenterCalc / Vertices.Num();
		Bounds = BoundingBox;
		bHasBounds = true;
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
	FDynamicMesh3 CreateDynamicMesh(bool bProcessMesh = false);

private:
	bool CheckConvex();
	bool CheckNonTris();
	bool CheckBounds() const;

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