// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMeshUtils.h"

#include "NCoreMinimal.h"
#include "Chaos/Convex.h"
#include "Math/NTriangleUtils.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMeshFactory.h"

// #SONARQUBE-DISABLE

void FNRawMeshUtils::CombineMesh(const FTransform& BaseTransform, FNRawMesh& BaseMesh,
                                 const FTransform& OtherTransform, const FNRawMesh& OtherMesh)
{
	if (OtherMesh.Vertices.IsEmpty() || OtherMesh.Loops.IsEmpty())
	{
		return;
	}

	// Pre-compose Other-local → Base-local once so the per-vertex hot loop is a single TransformPosition call.
	const FTransform OtherToBase = OtherTransform * BaseTransform.Inverse();

	const int32 VertexOffset = BaseMesh.Vertices.Num();

	BaseMesh.Vertices.Reserve(VertexOffset + OtherMesh.Vertices.Num());
	for (const FVector& OtherVertex : OtherMesh.Vertices)
	{
		BaseMesh.Vertices.Add(OtherToBase.TransformPosition(OtherVertex));
	}

	BaseMesh.Loops.Reserve(BaseMesh.Loops.Num() + OtherMesh.Loops.Num());
	for (const FNRawMeshLoop& OtherLoop : OtherMesh.Loops)
	{
		FNRawMeshLoop Shifted;
		Shifted.Indices.Reserve(OtherLoop.Indices.Num());
		for (const int32 Index : OtherLoop.Indices)
		{
			Shifted.Indices.Add(Index + VertexOffset);
		}
		BaseMesh.Loops.Add(MoveTemp(Shifted));
	}

	// FaceLoops carries the polygonal pre-triangulation description used by CheckConvex. Preserve it only
	// when BOTH sides describe themselves polygonally; otherwise clear, because partial coverage would let
	// CheckConvex skip the undocumented faces and falsely accept non-convex merges. When both sides have
	// it, the combined description is still geometrically meaningful — for genuinely convex merges (two
	// hulls along a shared face) CheckConvex correctly accepts, and for unrelated stitches it correctly
	// rejects because each side's vertices land on the wrong side of the other side's face planes.
	if (BaseMesh.FaceLoops.Num() > 0 && OtherMesh.FaceLoops.Num() > 0)
	{
		BaseMesh.FaceLoops.Reserve(BaseMesh.FaceLoops.Num() + OtherMesh.FaceLoops.Num());
		for (const FNRawMeshLoop& OtherFace : OtherMesh.FaceLoops)
		{
			FNRawMeshLoop Shifted;
			Shifted.Indices.Reserve(OtherFace.Indices.Num());
			for (const int32 Index : OtherFace.Indices)
			{
				Shifted.Indices.Add(Index + VertexOffset);
			}
			BaseMesh.FaceLoops.Add(MoveTemp(Shifted));
		}
	}
	else
	{
		BaseMesh.FaceLoops.Reset();
	}

	// A merged mesh is no longer a single Chaos-cooked body; mark validation dirty so the next
	// consumer query (or an explicit Validate call) re-evaluates convexity and tri-ness. Marking
	// rather than re-running is the entire point of lazy validation — a tight CombineMesh loop
	// would otherwise pay O(V * F) per merge for a convexity check the caller may never need.
	BaseMesh.bIsChaosGenerated = false;
	BaseMesh.InvalidateCachedFacePlanes();

	BaseMesh.CalculateCenterAndBounds();
	BaseMesh.InvalidateValidation();
}

bool FNRawMeshUtils::DoesIntersect(const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
                                   const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	// Quick rotated AABB check to early out if possible
	if (LeftMesh.HasBounds() && RightMesh.HasBounds())
	{
		const FBox LeftWorldBounds = LeftMesh.Bounds.TransformBy(FTransform(LeftRotation, LeftOrigin));
		const FBox RightWorldBounds = RightMesh.Bounds.TransformBy(FTransform(RightRotation, RightOrigin));
		if (!LeftWorldBounds.Intersect(RightWorldBounds))
		{
			return false;
		}
	}

	if (LeftMesh.Loops.IsEmpty() || RightMesh.Loops.IsEmpty())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMeshes, unable to determine if there is any intersection."));
		return false;
	}
	if (LeftMesh.HasNonTris() || RightMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("One or both of the provided FNRawMeshes has non-triangle based geometry (NGons), this is not supported for intersection checks."));
		return false;
	}

	// Convex fast path: SAT on the polygonal face description. Decisive for primitives / Chaos-built
	// hulls because face-normal axes typically separate non-overlapping pairs after only a handful of
	// projections. Falls through to the tri-tri sweep when SAT's preconditions aren't met or the meshes
	// are large enough that SAT's O(F^2) edge-cross worst case would beat us.
	if (CanUseSAT(LeftMesh, RightMesh))
	{
		return DoesConvexIntersectSAT(LeftMesh, LeftOrigin, LeftRotation, RightMesh, RightOrigin, RightRotation);
	}

	return DoesIntersectTriangles(LeftMesh, LeftOrigin, LeftRotation, RightMesh, RightOrigin, RightRotation);
}

bool FNRawMeshUtils::DoesIntersect(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	// LeftMesh has already had its transform baked into its vertices (and Bounds), so it lives in the world
	// frame; RightMesh is still local and is placed by RightOrigin/RightRotation. Delegate to the full
	// pipeline with an identity transform on the left — every left-side TransformPoint / RotateVector becomes
	// a no-op against the already-world-space data, while the right side is transformed as supplied.
	return DoesIntersect(LeftMesh, FVector::ZeroVector, FRotator::ZeroRotator,
	                     RightMesh, RightOrigin, RightRotation);
}

bool FNRawMeshUtils::DoesIntersect(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh)
{
	// Both meshes have already had their transform baked into their vertices (and Bounds), so they share
	// the world frame — this is the hot path for the World Assembly graph builder. Test the world-space
	// data directly instead of delegating through identity transforms, which would re-derive the same
	// answer via a redundant Bounds.TransformBy plus per-vertex identity rotations. Bit-identical to that
	// delegation because identity rotate / zero translate reduce to the stored vertices exactly.
	if (LeftMesh.HasBounds() && RightMesh.HasBounds())
	{
		if (!LeftMesh.Bounds.Intersect(RightMesh.Bounds))
		{
			return false;
		}
	}

	if (LeftMesh.Loops.IsEmpty() || RightMesh.Loops.IsEmpty())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMeshes, unable to determine if there is any intersection."));
		return false;
	}
	if (LeftMesh.HasNonTris() || RightMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("One or both of the provided FNRawMeshes has non-triangle based geometry (NGons), this is not supported for intersection checks."));
		return false;
	}

	if (CanUseSAT(LeftMesh, RightMesh))
	{
		return DoesConvexIntersectSATBaked(LeftMesh, RightMesh);
	}

	return DoesIntersectTrianglesBaked(LeftMesh, RightMesh);
}

float FNRawMeshUtils::GetIntersectDepth(
	const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
	const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation,
	const float EarlyExitDepth)
{
	// Rotated AABB early-out, mirroring DoesIntersect. The world-space AABBs computed here also feed
	// the C (AABB-overlap depth bound) and B (per-vertex AABB rejection) optimizations below.
	FBox LeftWorldBounds(ForceInit);
	FBox RightWorldBounds(ForceInit);
	if (LeftMesh.HasBounds() && RightMesh.HasBounds())
	{
		LeftWorldBounds = LeftMesh.Bounds.TransformBy(FTransform(LeftRotation, LeftOrigin));
		RightWorldBounds = RightMesh.Bounds.TransformBy(FTransform(RightRotation, RightOrigin));
		if (!LeftWorldBounds.Intersect(RightWorldBounds))
		{
			return -1.0f;
		}

		// C: AABB-overlap depth bound. When the overlap is a partial slice of both AABBs, the
		// narrowest overlap axis bounds how deep any vertex can sit inside the other mesh. If that
		// bound is below the caller's threshold, skip the per-vertex loops. NOT valid when one AABB
		// fully contains the other — the narrow dimension is just the smaller mesh's extent, not a
		// penetration bound.
		if (EarlyExitDepth < MAX_flt)
		{
			const bool bOneContainsOther = LeftWorldBounds.IsInside(RightWorldBounds)
				|| RightWorldBounds.IsInside(LeftWorldBounds);
			if (!bOneContainsOther)
			{
				const FBox Overlap = LeftWorldBounds.Overlap(RightWorldBounds);
				if (Overlap.IsValid)
				{
					const FVector OverlapSize = Overlap.GetExtent() * 2.0;
					const float MaxPossibleDepth = static_cast<float>(FMath::Min3(OverlapSize.X, OverlapSize.Y, OverlapSize.Z));
					if (MaxPossibleDepth < EarlyExitDepth)
					{
						return MaxPossibleDepth;
					}
				}
			}
		}
	}

	if (LeftMesh.Loops.IsEmpty() || RightMesh.Loops.IsEmpty())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMeshes, unable to determine intersection depth."));
		return -1.0f;
	}
	if (LeftMesh.HasNonTris() || RightMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("One or both of the provided FNRawMeshes has non-triangle based geometry (NGons), this is not supported for intersection depth checks."));
		return -1.0f;
	}
	// Convex requirement dropped — the non-convex path (parity-raycast containment + point-to-triangle
	// surface distance) is plumbed through ComputePointDepthInside. Convex inputs still take the existing
	// face-plane fast path inside that dispatch.

	const FQuat LeftQuat = LeftRotation.Quaternion();
	const FQuat RightQuat = RightRotation.Quaternion();
	const FQuat LeftInvQuat = LeftQuat.Inverse();
	const FQuat RightInvQuat = RightQuat.Inverse();

	float MaxDepth = 0.0f;

	// Right vertices, expressed in Left's local space, tested against Left's hull. B: skip vertices
	// that are clearly outside Left's world AABB — they can't be inside Left's hull and would just
	// pay the per-face ComputePointDepthInside sweep to return -1.
	for (const FVector& V : RightMesh.Vertices)
	{
		const FVector WorldPos = RightQuat.RotateVector(V) + RightOrigin;
		if (LeftWorldBounds.IsValid && !LeftWorldBounds.IsInsideOrOn(WorldPos))
		{
			continue;
		}
		const FVector LeftLocal = LeftInvQuat.RotateVector(WorldPos - LeftOrigin);
		const float Depth = ComputePointDepthInside(LeftMesh, LeftLocal);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
			// A: in-loop early-exit once the running max exceeds the caller's threshold.
			if (MaxDepth >= EarlyExitDepth)
			{
				return MaxDepth;
			}
		}
	}

	// Left vertices, expressed in Right's local space, tested against Right's hull. Same B + A treatment.
	for (const FVector& V : LeftMesh.Vertices)
	{
		const FVector WorldPos = LeftQuat.RotateVector(V) + LeftOrigin;
		if (RightWorldBounds.IsValid && !RightWorldBounds.IsInsideOrOn(WorldPos))
		{
			continue;
		}
		const FVector RightLocal = RightInvQuat.RotateVector(WorldPos - RightOrigin);
		const float Depth = ComputePointDepthInside(RightMesh, RightLocal);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
			if (MaxDepth >= EarlyExitDepth)
			{
				return MaxDepth;
			}
		}
	}

	return MaxDepth;
}

float FNRawMeshUtils::GetIntersectDepth(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation, float EarlyExitDepth)
{
	// LeftMesh has already had its transform baked into its vertices (and Bounds), so it lives in the world
	// frame; RightMesh is still local and is placed by RightOrigin/RightRotation. Delegate to the full metric
	// with an identity transform on the left — every left-side TransformPoint / RotateVector becomes a no-op
	// against the already-world-space data, while the right side is transformed as supplied.
	return GetIntersectDepth(LeftMesh, FVector::ZeroVector, FRotator::ZeroRotator,
	                        RightMesh, RightOrigin, RightRotation, EarlyExitDepth);
}

float FNRawMeshUtils::GetIntersectDepth(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh, float EarlyExitDepth)
{
	// Both meshes are baked into world space — the dominant per-placement metric in the graph builder.
	// Use the stored world-space Bounds and vertices directly rather than delegating through identity
	// transforms, which would otherwise pay a redundant Bounds.TransformBy and two quaternion rotations per
	// vertex in the hottest loop. Bit-identical to that delegation: identity rotate / zero translate leave
	// each vertex unchanged, and Bounds.TransformBy(identity) re-encloses the same corners.
	FBox LeftWorldBounds(ForceInit);
	FBox RightWorldBounds(ForceInit);
	if (LeftMesh.HasBounds() && RightMesh.HasBounds())
	{
		LeftWorldBounds = LeftMesh.Bounds;
		RightWorldBounds = RightMesh.Bounds;
		if (!LeftWorldBounds.Intersect(RightWorldBounds))
		{
			return -1.0f;
		}

		// C: AABB-overlap depth bound — see the transform-aware overload for the full rationale.
		if (EarlyExitDepth < MAX_flt)
		{
			const bool bOneContainsOther = LeftWorldBounds.IsInside(RightWorldBounds)
				|| RightWorldBounds.IsInside(LeftWorldBounds);
			if (!bOneContainsOther)
			{
				const FBox Overlap = LeftWorldBounds.Overlap(RightWorldBounds);
				if (Overlap.IsValid)
				{
					const FVector OverlapSize = Overlap.GetExtent() * 2.0;
					const float MaxPossibleDepth = static_cast<float>(FMath::Min3(OverlapSize.X, OverlapSize.Y, OverlapSize.Z));
					if (MaxPossibleDepth < EarlyExitDepth)
					{
						return MaxPossibleDepth;
					}
				}
			}
		}
	}

	if (LeftMesh.Loops.IsEmpty() || RightMesh.Loops.IsEmpty())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMeshes, unable to determine intersection depth."));
		return -1.0f;
	}
	if (LeftMesh.HasNonTris() || RightMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("One or both of the provided FNRawMeshes has non-triangle based geometry (NGons), this is not supported for intersection depth checks."));
		return -1.0f;
	}

	float MaxDepth = 0.0f;

	// Right vertices tested against Left's hull. B: skip vertices clearly outside Left's world AABB.
	for (const FVector& V : RightMesh.Vertices)
	{
		if (LeftWorldBounds.IsValid && !LeftWorldBounds.IsInsideOrOn(V))
		{
			continue;
		}
		const float Depth = ComputePointDepthInside(LeftMesh, V);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
			if (MaxDepth >= EarlyExitDepth)
			{
				return MaxDepth;
			}
		}
	}

	// Left vertices tested against Right's hull. Same B + A treatment.
	for (const FVector& V : LeftMesh.Vertices)
	{
		if (RightWorldBounds.IsValid && !RightWorldBounds.IsInsideOrOn(V))
		{
			continue;
		}
		const float Depth = ComputePointDepthInside(RightMesh, V);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
			if (MaxDepth >= EarlyExitDepth)
			{
				return MaxDepth;
			}
		}
	}

	return MaxDepth;
}

float FNRawMeshUtils::GetIntersectDepth(const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation, const FVector& WorldPosition)
{
	if (LeftMesh.HasBounds())
	{
		const FBox LeftWorldBounds = LeftMesh.Bounds.TransformBy(FTransform(LeftRotation, LeftOrigin));
		if (!LeftWorldBounds.IsInsideOrOn(WorldPosition))
		{
			return -1.0f;
		}
	}

	if (LeftMesh.Loops.IsEmpty())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMesh, unable to determine intersection depth."));
		return -1.0f;
	}
	if (LeftMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("The provided FNRawMesh has non-triangle based geometry (NGons), this is not supported for intersection depth checks."));
		return -1.0f;
	}

	const FVector LeftLocal = LeftRotation.Quaternion().Inverse().RotateVector(WorldPosition - LeftOrigin);
	return ComputePointDepthInside(LeftMesh, LeftLocal);
}

float FNRawMeshUtils::GetIntersectDepth(const FNRawMesh& LeftMesh, const FVector& WorldPosition)
{
	// LeftMesh has already had its transform baked into its vertices (and Bounds), so it lives in the world
	// frame and WorldPosition can be tested against it directly. Delegate to the transform-aware point-depth
	// query with an identity transform — the local-space conversion of WorldPosition reduces to the identity,
	// so the point is measured against the mesh as-is.
	return GetIntersectDepth(LeftMesh, FVector::ZeroVector, FRotator::ZeroRotator, WorldPosition);
}

TArray<ANDebugActor*> FNRawMeshUtils::CreateRawMeshVisualizers(UWorld* World, const TArray<FNRawMesh>& Meshes, const TArray<FTransform>& Transforms,  UMaterialInterface* MaterialInterface, bool bSingleActor, bool bProcessMeshes)
{
	TArray<ANDebugActor*> DebugActors;

	if (bSingleActor)
	{
		FNRawMesh BaseMesh;
		FTransform BaseTransform;
		for (int32 i = 0; i < Transforms.Num(); i++)
		{
			CombineMesh(BaseTransform, BaseMesh, Transforms[i], Meshes[i]);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = MakeUniqueObjectName(World, ANDebugActor::StaticClass(), FName("RawMeshVisualizer"));
#if WITH_EDITOR
		const FString Label = SpawnParams.Name.ToString();
		SpawnParams.InitialActorLabel = Label;
#endif // WITH_EDITOR
		SpawnParams.ObjectFlags |= RF_Transient;

		ANDebugActor* DebugActor = World->SpawnActor<ANDebugActor>(ANDebugActor::StaticClass(), BaseTransform, SpawnParams);
		if (DebugActor == nullptr) return DebugActors;

		DebugActor->OverrideWithDynamicMesh(BaseMesh.CreateDynamicMesh(bProcessMeshes), MaterialInterface);
		DebugActors.Add(DebugActor);
	}
	else
	{
		for (int32 i = 0; i < Transforms.Num(); i++)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = MakeUniqueObjectName(World, ANDebugActor::StaticClass(), FName("RawMeshVisualizer"));
	#if WITH_EDITOR
			const FString Label = SpawnParams.Name.ToString();
			SpawnParams.InitialActorLabel = Label;
	#endif // WITH_EDITOR
			SpawnParams.ObjectFlags |= RF_Transient;

			ANDebugActor* DebugActor = World->SpawnActor<ANDebugActor>(ANDebugActor::StaticClass(), Transforms[i], SpawnParams);
			if (DebugActor == nullptr) continue;

			DebugActor->OverrideWithDynamicMesh(Meshes[i].CreateDynamicMesh(bProcessMeshes), MaterialInterface);
			DebugActors.Add(DebugActor);
		}
	}
	return DebugActors;
}

FNRawMesh FNRawMeshUtils::ToConvexHull(const FNRawMesh& Mesh)
{
	if (Mesh.IsConvex())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("Converting mesh to ConvexHull that is already a convex hull. This duplicates the mesh!!!"));
		return Mesh;
	}

	FNRawMesh Result;
	if (Mesh.Vertices.Num() < 4)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("Cannot build a convex hull from fewer than 4 vertices; returning empty mesh."));
		return Result;
	}

	// Feed the source vertex cloud into Chaos's hull builder; the source loops are discarded.
	TArray<Chaos::FConvex::FVec3Type> InputVertices;
	InputVertices.Reserve(Mesh.Vertices.Num());
	for (const FVector& Vertex : Mesh.Vertices)
	{
		InputVertices.Add(Chaos::FConvex::FVec3Type(Vertex));
	}

	TArray<Chaos::FConvex::FPlaneType> OutPlanes;
	TArray<TArray<int32>> OutFaceIndices;
	TArray<Chaos::FConvex::FVec3Type> OutVertices;
	Chaos::FConvex::FAABB3Type OutLocalBounds;
	Chaos::FConvexBuilder::Build(InputVertices, OutPlanes, OutFaceIndices, OutVertices, OutLocalBounds,
		Chaos::FConvexBuilder::EBuildMethod::Original); // Use the original
	const int32 VerticesCount = OutVertices.Num();
	const int32 LoopCount = OutFaceIndices.Num();
	if (VerticesCount == 0 || LoopCount == 0)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("Convex hull build returned no geometry; returning empty mesh."));
		return Result;
	}

	Result.Vertices.Reserve(VerticesCount);
	FVector CenterCalc = FVector::ZeroVector;
	FBox BoundingBox(ForceInit);
	for (int32 i = 0; i < VerticesCount; i++)
	{
		const FVector Created(OutVertices[i][0], OutVertices[i][1], OutVertices[i][2]);
		Result.Vertices.Add(Created);
		CenterCalc += Created;
		BoundingBox += Created;
	}

	// Capture the polygonal faces straight from Chaos as the canonical face description, then start Loops
	// as a copy so triangulation has something to work on. FaceLoops remains polygonal and is what
	// CheckConvex consults — that avoids the per-triangle coplanar-drift false-negative after a vertex edit.
	Result.FaceLoops.Reserve(LoopCount);
	for (int32 i = 0; i < LoopCount; i++)
	{
		Result.FaceLoops.Add(FNRawMeshLoop(OutFaceIndices[i]));
	}
	Result.Loops = Result.FaceLoops;

	Result.Center = CenterCalc / VerticesCount;
	Result.Bounds = BoundingBox;

	// Trust the Chaos output: faces may be n-gons but are convex by construction.
	Result.bIsChaosGenerated = true;
	Result.bIsConvex = true;
	Result.bHasBounds = true;

	// ConvertToTriangles self-early-outs when Loops is already all-tris, so call it unconditionally and
	// skip the extra CheckNonTris pass that used to gate it.
	Result.ConvertToTriangles();
	Result.bHasNonTris = false;

	return Result;
}

FNRawMesh FNRawMeshUtils::MakeBoxHull(const FBox& Box)
{
	const FVector Min = Box.Min;
	const FVector Max = Box.Max;

	FNRawMesh Hull;
	Hull.Vertices = {
		FVector(Min.X, Min.Y, Min.Z), // 0
		FVector(Max.X, Min.Y, Min.Z), // 1
		FVector(Max.X, Max.Y, Min.Z), // 2
		FVector(Min.X, Max.Y, Min.Z), // 3
		FVector(Min.X, Min.Y, Max.Z), // 4
		FVector(Max.X, Min.Y, Max.Z), // 5
		FVector(Max.X, Max.Y, Max.Z), // 6
		FVector(Min.X, Max.Y, Max.Z), // 7
	};

	// Polygonal faces, each wound so its Newell normal points away from the box interior.
	Hull.FaceLoops = {
		FNRawMeshLoop(0, 3, 2, 1), // -Z bottom
		FNRawMeshLoop(4, 5, 6, 7), // +Z top
		FNRawMeshLoop(0, 1, 5, 4), // -Y front
		FNRawMeshLoop(1, 2, 6, 5), // +X right
		FNRawMeshLoop(2, 3, 7, 6), // +Y back
		FNRawMeshLoop(3, 0, 4, 7), // -X left
	};

	// Matching fan triangulation (a,b,c,d -> a,b,c + a,c,d) preserving each face's outward winding.
	Hull.Loops = {
		FNRawMeshLoop(0, 3, 2), FNRawMeshLoop(0, 2, 1),
		FNRawMeshLoop(4, 5, 6), FNRawMeshLoop(4, 6, 7),
		FNRawMeshLoop(0, 1, 5), FNRawMeshLoop(0, 5, 4),
		FNRawMeshLoop(1, 2, 6), FNRawMeshLoop(1, 6, 5),
		FNRawMeshLoop(2, 3, 7), FNRawMeshLoop(2, 7, 6),
		FNRawMeshLoop(3, 0, 4), FNRawMeshLoop(3, 4, 7),
	};

	Hull.CalculateCenterAndBounds();
	// Eagerly compute the convexity / non-tri flags. The buffers were assigned directly (bypassing the
	// mutators that would mark validation dirty), so without this the cached flags stay at their stale
	// defaults and IsConvex() would report false until some later mutation invalidated them.
	Hull.Validate();
	return Hull;
}

bool FNRawMeshUtils::IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint)
{
	if (Mesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The FNRawMesh has non-triangle loops; unable to determine IsRelativePointInside; returning false."));
		return false;
	}

	if (Mesh.IsConvex())
	{
		// Convex fast path: for every face plane, the test point must lie on the same side as the mesh
		// centroid. Reads the cached face-plane data (Normal + PlaneD precomputed once per mesh edit)
		// so each query is two dot products per face — no Cross, no per-vertex subtraction.
		const TArray<FVector>& Normals = Mesh.GetCachedFaceNormals();
		const TArray<double>& PlaneDs = Mesh.GetCachedFacePlaneD();
		const int32 LoopCount = Mesh.Loops.Num();
		for (int32 i = 0; i < LoopCount; ++i)
		{
			const FVector& Normal = Normals[i];
			const double PlaneD = PlaneDs[i];
			const double CenterSide = FVector::DotProduct(Normal, Mesh.Center) - PlaneD;
			const double PointSide  = FVector::DotProduct(Normal, RelativePoint) - PlaneD;

			if (CenterSide * PointSide < 0.0)
			{
				return false;
			}
		}

		return true;
	}

	// Non-convex fallback: odd-parity ray cast. Cast a ray in a non-axis-aligned direction so it
	// doesn't graze the axis-aligned edges typical of editor hulls, count strict-interior triangle
	// crossings, and an odd total means the point is enclosed by the closed surface. Assumes the
	// mesh is a closed manifold — which a SplitEdge-edited hull remains, since SplitEdge only
	// subdivides existing edges and never opens the surface.
	//
	// Direction picked to be clearly irrational and non-symmetric so it doesn't align with any
	// hull edge or face normal that the user might construct (axis planes, 45° diagonals, etc.).
	// Magnitude is irrelevant; only the sign of t and the barycentric inequalities matter.
	static const FVector ProbeDirection(0.832050, 0.416025, 0.366092);

	int32 HitCount = 0;
	for (const FNRawMeshLoop& Loop : Mesh.Loops)
	{
		const FVector& A = Mesh.Vertices[Loop.Indices[0]];
		const FVector& B = Mesh.Vertices[Loop.Indices[1]];
		const FVector& C = Mesh.Vertices[Loop.Indices[2]];
		if (FNTriangleUtils::RayIntersectsTriangle(RelativePoint, ProbeDirection, A, B, C))
		{
			++HitCount;
		}
	}

	return (HitCount & 1) == 1;
}

bool FNRawMeshUtils::AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints)
{
	if (Mesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The FNRawMesh has non-triangle loops; unable to determine AnyRelativePointsInside; returning false."));
		return false;
	}

	for (const FVector& Point : RelativePoints)
	{
		if (IsRelativePointInside(Mesh, Point))
		{
			return true;
		}
	}
	return false;
}

float FNRawMeshUtils::ComputePointDepthInside(const FNRawMesh& Mesh, const FVector& RelativePoint)
{
	if (Mesh.IsConvex())
	{
		// Convex hull: face planes bound the body, so max plane-distance gives the depth in one pass
		// and naturally returns -1 when the point lies outside any half-space.
		return ComputePointDepthInsideConvex(Mesh, RelativePoint);
	}

	// Non-convex: containment isn't decidable from face planes alone (a face's infinite plane may have
	// body on both sides of it). Run the parity-raycast containment probe first, then measure distance
	// to the actual triangle surface — point-to-plane would over-estimate by ignoring the triangle's
	// finite extent.
	if (!IsRelativePointInside(Mesh, RelativePoint))
	{
		return -1.0f;
	}
	return ComputePointDepthInsideNonConvex(Mesh, RelativePoint);
}

float FNRawMeshUtils::ComputePointDepthInsideNonConvex(const FNRawMesh& Mesh, const FVector& RelativePoint)
{
	// Minimum distance from RelativePoint to any triangle's surface. Assumes the caller has already
	// verified the point is inside the closed manifold; this function just measures distance-to-surface.
	double MinDist = MAX_dbl;
	for (const FNRawMeshLoop& Loop : Mesh.Loops)
	{
		const FVector& A = Mesh.Vertices[Loop.Indices[0]];
		const FVector& B = Mesh.Vertices[Loop.Indices[1]];
		const FVector& C = Mesh.Vertices[Loop.Indices[2]];
		const double D = FNTriangleUtils::DistanceFromPointToTriangle(RelativePoint, A, B, C);
		if (D < MinDist)
		{
			MinDist = D;
		}
	}
	return (MinDist == MAX_dbl) ? 0.0f : static_cast<float>(MinDist);
}

float FNRawMeshUtils::ComputePointDepthInsideConvex(const FNRawMesh& Mesh, const FVector& RelativePoint)
{
	// Cached face planes turn each per-face query into two dot products + a precomputed multiply,
	// eliminating the per-face Cross / Sqrt that used to dominate this loop. Degenerate triangles
	// get InvNormalLen == 0 at cache-build time and naturally contribute no perpendicular distance.
	const TArray<FVector>& Normals = Mesh.GetCachedFaceNormals();
	const TArray<double>& PlaneDs = Mesh.GetCachedFacePlaneD();
	const TArray<double>& InvLens = Mesh.GetCachedFaceInvNormalLen();
	const int32 LoopCount = Mesh.Loops.Num();

	float MinFaceDist = MAX_flt;
	for (int32 i = 0; i < LoopCount; ++i)
	{
		const double InvLen = InvLens[i];
		if (InvLen == 0.0)
		{
			continue;
		}

		const FVector& Normal = Normals[i];
		const double PlaneD = PlaneDs[i];
		const double CenterSide = FVector::DotProduct(Normal, Mesh.Center) - PlaneD;
		const double PointSide  = FVector::DotProduct(Normal, RelativePoint) - PlaneD;

		if (CenterSide * PointSide < 0.0)
		{
			return -1.0f;
		}

		const double PerpDist = FMath::Abs(PointSide) * InvLen;
		if (PerpDist < MinFaceDist)
		{
			MinFaceDist = static_cast<float>(PerpDist);
		}
	}
	return (MinFaceDist == MAX_flt) ? 0.0f : MinFaceDist;
}

bool FNRawMeshUtils::DoesIntersectTriangles(const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
                                            const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	// Transform vertices of both meshes to world space
	TArray<FVector> LeftVerticesWorld;
	LeftVerticesWorld.Reserve(LeftMesh.Vertices.Num());
	for (const FVector& Vertex : LeftMesh.Vertices)
	{
		LeftVerticesWorld.Add(FNVectorUtils::TransformPoint(Vertex, LeftOrigin, LeftRotation));
	}

	TArray<FVector> RightVerticesWorld;
	RightVerticesWorld.Reserve(RightMesh.Vertices.Num());
	for (const FVector& Vertex : RightMesh.Vertices)
	{
		RightVerticesWorld.Add(FNVectorUtils::TransformPoint(Vertex, RightOrigin, RightRotation));
	}

	const int32 LeftLoopCount = LeftMesh.Loops.Num();
	const int32 RightLoopCount = RightMesh.Loops.Num();

	// Precompute world-space per-triangle AABBs so the inner loop pays a 6-compare overlap rejection
	// instead of the full Möller test on every pair. Adds O(L+R) build cost amortized across O(L*R)
	// tests — a clear win on any hull with more than a few triangles, and decisive on the worst case
	// (AABB-hit-mesh-miss) where every Möller call previously ran to completion only to fail.
	TArray<FBox> LeftTriBounds;
	LeftTriBounds.SetNumUninitialized(LeftLoopCount);
	for (int32 i = 0; i < LeftLoopCount; ++i)
	{
		if (!LeftMesh.Loops[i].IsTriangle())
		{
			LeftTriBounds[i] = FBox(ForceInit);
			continue;
		}
		const FVector& V0 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[0]];
		const FVector& V1 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[1]];
		const FVector& V2 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[2]];
		LeftTriBounds[i] = FBox(FVector::Min(FVector::Min(V0, V1), V2), FVector::Max(FVector::Max(V0, V1), V2));
	}

	TArray<FBox> RightTriBounds;
	RightTriBounds.SetNumUninitialized(RightLoopCount);
	for (int32 j = 0; j < RightLoopCount; ++j)
	{
		if (!RightMesh.Loops[j].IsTriangle())
		{
			RightTriBounds[j] = FBox(ForceInit);
			continue;
		}
		const FVector& U0 = RightVerticesWorld[RightMesh.Loops[j].Indices[0]];
		const FVector& U1 = RightVerticesWorld[RightMesh.Loops[j].Indices[1]];
		const FVector& U2 = RightVerticesWorld[RightMesh.Loops[j].Indices[2]];
		RightTriBounds[j] = FBox(FVector::Min(FVector::Min(U0, U1), U2), FVector::Max(FVector::Max(U0, U1), U2));
	}

	for (int32 i = 0; i < LeftLoopCount; ++i)
	{
		if (!LeftMesh.Loops[i].IsTriangle()) continue;
		const FBox& LeftTriBox = LeftTriBounds[i];

		const FVector& V0 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[0]];
		const FVector& V1 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[1]];
		const FVector& V2 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[2]];

		for (int32 j = 0; j < RightLoopCount; ++j)
		{
			if (!RightMesh.Loops[j].IsTriangle()) continue;
			if (!LeftTriBox.Intersect(RightTriBounds[j])) continue;

			const FVector& U0 = RightVerticesWorld[RightMesh.Loops[j].Indices[0]];
			const FVector& U1 = RightVerticesWorld[RightMesh.Loops[j].Indices[1]];
			const FVector& U2 = RightVerticesWorld[RightMesh.Loops[j].Indices[2]];

			if (FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2))
			{
				return true;
			}
		}
	}

	// Triangle-triangle tests only catch surface crossings. If one mesh is entirely contained within
	// the other, no triangles will intersect but the meshes still overlap. Gate the (relatively
	// expensive) IsRelativePointInside probes on whether enclosure is geometrically possible at all:
	// transform each mesh's local Bounds into world space and check whether one's AABB fully contains
	// the other's. If neither contains the other, no mesh enclosure can occur and both probes are
	// skipped — this is the common case for equal-sized cells in the World Assembly graph builder.
	auto BoxAContainsBoxB = [](const FBox& A, const FBox& B) -> bool
	{
		return A.Min.X <= B.Min.X && B.Max.X <= A.Max.X
			&& A.Min.Y <= B.Min.Y && B.Max.Y <= A.Max.Y
			&& A.Min.Z <= B.Min.Z && B.Max.Z <= A.Max.Z;
	};

	const FBox LeftWorldBounds = LeftMesh.HasBounds()
		? LeftMesh.Bounds.TransformBy(FTransform(LeftRotation, LeftOrigin))
		: FBox(ForceInit);
	const FBox RightWorldBounds = RightMesh.HasBounds()
		? RightMesh.Bounds.TransformBy(FTransform(RightRotation, RightOrigin))
		: FBox(ForceInit);

	const bool bLeftCouldBeInsideRight = LeftWorldBounds.IsValid && RightWorldBounds.IsValid
		&& BoxAContainsBoxB(RightWorldBounds, LeftWorldBounds);
	const bool bRightCouldBeInsideLeft = LeftWorldBounds.IsValid && RightWorldBounds.IsValid
		&& BoxAContainsBoxB(LeftWorldBounds, RightWorldBounds);

	// Note: to put a world-space point into mesh M's local space, we invert M's rotation. The original
	// code here inverted the source mesh's rotation by mistake (LeftInvQuat applied to a sample we then
	// tested against RightMesh), which produced wrong containment results whenever Left and Right had
	// different rotations. Both probes below now use the destination mesh's inverse rotation.
	if (bLeftCouldBeInsideRight)
	{
		const FQuat RightInvQuat = RightRotation.Quaternion().Inverse();
		const FVector LeftSampleInRightLocal = RightInvQuat.RotateVector(LeftVerticesWorld[0] - RightOrigin);
		if (IsRelativePointInside(RightMesh, LeftSampleInRightLocal))
		{
			return true;
		}
	}

	if (bRightCouldBeInsideLeft)
	{
		const FQuat LeftInvQuat = LeftRotation.Quaternion().Inverse();
		const FVector RightSampleInLeftLocal = LeftInvQuat.RotateVector(RightVerticesWorld[0] - LeftOrigin);
		if (IsRelativePointInside(LeftMesh, RightSampleInLeftLocal))
		{
			return true;
		}
	}

	return false;
}

bool FNRawMeshUtils::DoesIntersectTrianglesBaked(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh)
{
	// Vertices are already world-space (baked), so reference the stored arrays directly instead of
	// allocating and filling Left/RightVerticesWorld the way the transform-aware path must.
	const TArray<FVector>& LeftVerticesWorld = LeftMesh.Vertices;
	const TArray<FVector>& RightVerticesWorld = RightMesh.Vertices;

	const int32 LeftLoopCount = LeftMesh.Loops.Num();
	const int32 RightLoopCount = RightMesh.Loops.Num();

	// Precompute per-triangle AABBs so the inner loop pays a 6-compare overlap rejection instead of the
	// full Möller test on every pair — mirrors the transform-aware sweep.
	TArray<FBox> LeftTriBounds;
	LeftTriBounds.SetNumUninitialized(LeftLoopCount);
	for (int32 i = 0; i < LeftLoopCount; ++i)
	{
		if (!LeftMesh.Loops[i].IsTriangle())
		{
			LeftTriBounds[i] = FBox(ForceInit);
			continue;
		}
		const FVector& V0 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[0]];
		const FVector& V1 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[1]];
		const FVector& V2 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[2]];
		LeftTriBounds[i] = FBox(FVector::Min(FVector::Min(V0, V1), V2), FVector::Max(FVector::Max(V0, V1), V2));
	}

	TArray<FBox> RightTriBounds;
	RightTriBounds.SetNumUninitialized(RightLoopCount);
	for (int32 j = 0; j < RightLoopCount; ++j)
	{
		if (!RightMesh.Loops[j].IsTriangle())
		{
			RightTriBounds[j] = FBox(ForceInit);
			continue;
		}
		const FVector& U0 = RightVerticesWorld[RightMesh.Loops[j].Indices[0]];
		const FVector& U1 = RightVerticesWorld[RightMesh.Loops[j].Indices[1]];
		const FVector& U2 = RightVerticesWorld[RightMesh.Loops[j].Indices[2]];
		RightTriBounds[j] = FBox(FVector::Min(FVector::Min(U0, U1), U2), FVector::Max(FVector::Max(U0, U1), U2));
	}

	for (int32 i = 0; i < LeftLoopCount; ++i)
	{
		if (!LeftMesh.Loops[i].IsTriangle()) continue;
		const FBox& LeftTriBox = LeftTriBounds[i];

		const FVector& V0 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[0]];
		const FVector& V1 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[1]];
		const FVector& V2 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[2]];

		for (int32 j = 0; j < RightLoopCount; ++j)
		{
			if (!RightMesh.Loops[j].IsTriangle()) continue;
			if (!LeftTriBox.Intersect(RightTriBounds[j])) continue;

			const FVector& U0 = RightVerticesWorld[RightMesh.Loops[j].Indices[0]];
			const FVector& U1 = RightVerticesWorld[RightMesh.Loops[j].Indices[1]];
			const FVector& U2 = RightVerticesWorld[RightMesh.Loops[j].Indices[2]];

			if (FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2))
			{
				return true;
			}
		}
	}

	// Containment fallback: bounds are already world-space, and a world-space point is already in each
	// baked mesh's own frame, so no inverse transform is needed on the sample point.
	auto BoxAContainsBoxB = [](const FBox& A, const FBox& B) -> bool
	{
		return A.Min.X <= B.Min.X && B.Max.X <= A.Max.X
			&& A.Min.Y <= B.Min.Y && B.Max.Y <= A.Max.Y
			&& A.Min.Z <= B.Min.Z && B.Max.Z <= A.Max.Z;
	};

	const FBox LeftWorldBounds = LeftMesh.HasBounds() ? LeftMesh.Bounds : FBox(ForceInit);
	const FBox RightWorldBounds = RightMesh.HasBounds() ? RightMesh.Bounds : FBox(ForceInit);

	const bool bLeftCouldBeInsideRight = LeftWorldBounds.IsValid && RightWorldBounds.IsValid
		&& BoxAContainsBoxB(RightWorldBounds, LeftWorldBounds);
	const bool bRightCouldBeInsideLeft = LeftWorldBounds.IsValid && RightWorldBounds.IsValid
		&& BoxAContainsBoxB(LeftWorldBounds, RightWorldBounds);

	if (bLeftCouldBeInsideRight)
	{
		if (IsRelativePointInside(RightMesh, LeftVerticesWorld[0]))
		{
			return true;
		}
	}

	if (bRightCouldBeInsideLeft)
	{
		if (IsRelativePointInside(LeftMesh, RightVerticesWorld[0]))
		{
			return true;
		}
	}

	return false;
}

bool FNRawMeshUtils::CanUseSAT(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh)
{
	// Empirical cap. SAT face/edge counts grow with FaceLoops.Num(); the worst-case (overlap) cost
	// scales O(F^2) on edge crosses. 32 keeps every primitive emitter (box=6, sphere/sphyl=tens of
	// faces) and typical artist-authored hulls inside the fast path while leaving dense static-mesh
	// hulls on the tri-tri sweep where they fare better.
	constexpr int32 MaxFacesForSAT = 32;
	return LeftMesh.IsConvex() && RightMesh.IsConvex()
		&& !LeftMesh.FaceLoops.IsEmpty() && !RightMesh.FaceLoops.IsEmpty()
		&& LeftMesh.FaceLoops.Num() <= MaxFacesForSAT
		&& RightMesh.FaceLoops.Num() <= MaxFacesForSAT;
}

bool FNRawMeshUtils::DoesConvexIntersectSAT(
	const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
	const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	const FQuat LeftQuat = LeftRotation.Quaternion();
	const FQuat RightQuat = RightRotation.Quaternion();

	// World-space vertex buffers — every axis test re-projects these, so transform once.
	TArray<FVector> LeftVertsWS;
	LeftVertsWS.Reserve(LeftMesh.Vertices.Num());
	for (const FVector& V : LeftMesh.Vertices)
	{
		LeftVertsWS.Add(LeftQuat.RotateVector(V) + LeftOrigin);
	}
	TArray<FVector> RightVertsWS;
	RightVertsWS.Reserve(RightMesh.Vertices.Num());
	for (const FVector& V : RightMesh.Vertices)
	{
		RightVertsWS.Add(RightQuat.RotateVector(V) + RightOrigin);
	}

	// Project both vertex buffers onto Axis and return true when the intervals overlap. The axis does
	// not need to be unit-length — interval comparisons survive scaling, and skipping the normalise
	// keeps the per-axis cost to two min/max reductions over the vertex sets.
	auto IntervalsOverlap = [&LeftVertsWS, &RightVertsWS](const FVector& Axis) -> bool
	{
		if (Axis.SizeSquared() < UE_KINDA_SMALL_NUMBER)
		{
			// Degenerate axis (zero cross product on parallel edges, or a degenerate face normal):
			// it can never separate the two hulls, so claim overlap and let another axis disprove it.
			return true;
		}
		double LMin = MAX_dbl;
		double LMax = -MAX_dbl;
		for (const FVector& V : LeftVertsWS)
		{
			const double D = FVector::DotProduct(Axis, V);
			if (D < LMin) LMin = D;
			if (D > LMax) LMax = D;
		}
		double RMin = MAX_dbl;
		double RMax = -MAX_dbl;
		for (const FVector& V : RightVertsWS)
		{
			const double D = FVector::DotProduct(Axis, V);
			if (D < RMin) RMin = D;
			if (D > RMax) RMax = D;
		}
		return LMax >= RMin && RMax >= LMin;
	};

	// 1. Left face normals — for axis-aligned / near-axis-aligned placements these usually find the
	//    separating axis in the first few iterations.
	for (const FNRawMeshLoop& Face : LeftMesh.FaceLoops)
	{
		if (Face.Indices.Num() < 3) continue;
		const FVector& V0 = LeftMesh.Vertices[Face.Indices[0]];
		const FVector& V1 = LeftMesh.Vertices[Face.Indices[1]];
		const FVector& V2 = LeftMesh.Vertices[Face.Indices[2]];
		const FVector LocalNormal = FVector::CrossProduct(V1 - V0, V2 - V0);
		const FVector WorldAxis = LeftQuat.RotateVector(LocalNormal);
		if (!IntervalsOverlap(WorldAxis))
		{
			return false;
		}
	}

	// 2. Right face normals.
	for (const FNRawMeshLoop& Face : RightMesh.FaceLoops)
	{
		if (Face.Indices.Num() < 3) continue;
		const FVector& V0 = RightMesh.Vertices[Face.Indices[0]];
		const FVector& V1 = RightMesh.Vertices[Face.Indices[1]];
		const FVector& V2 = RightMesh.Vertices[Face.Indices[2]];
		const FVector LocalNormal = FVector::CrossProduct(V1 - V0, V2 - V0);
		const FVector WorldAxis = RightQuat.RotateVector(LocalNormal);
		if (!IntervalsOverlap(WorldAxis))
		{
			return false;
		}
	}

	// 3. No face normal separated. Hand off to the tri-tri sweep instead of running the O(E_L * E_R)
	//    edge-cross axes — for the actual-overlap case (which is what reaches this point most often)
	//    tri-tri terminates on the first surface hit and beats SAT's exhaustive axis enumeration.
	//    The narrow case where edge-cross axes would have decided things (edge-on-edge near-touch
	//    with no surface intersection) falls through to tri-tri, which returns "no surface hit" plus
	//    "no enclosure via containment fallback" — the correct "no overlap" answer for that
	//    configuration. See the function-level doc comment for the safety argument.
	return DoesIntersectTriangles(LeftMesh, LeftOrigin, LeftRotation, RightMesh, RightOrigin, RightRotation);
}

bool FNRawMeshUtils::DoesConvexIntersectSATBaked(const FNRawMesh& LeftMesh, const FNRawMesh& RightMesh)
{
	// Vertices are already world-space (baked), so project them directly — no per-vertex rotation and no
	// world-vertex buffers. Face normals are likewise the cross product of the stored (world-space) edges,
	// which is exactly what the transform-aware path produces once the identity rotation is folded out.
	const TArray<FVector>& LeftVertsWS = LeftMesh.Vertices;
	const TArray<FVector>& RightVertsWS = RightMesh.Vertices;

	auto IntervalsOverlap = [&LeftVertsWS, &RightVertsWS](const FVector& Axis) -> bool
	{
		if (Axis.SizeSquared() < UE_KINDA_SMALL_NUMBER)
		{
			return true;
		}
		double LMin = MAX_dbl;
		double LMax = -MAX_dbl;
		for (const FVector& V : LeftVertsWS)
		{
			const double D = FVector::DotProduct(Axis, V);
			if (D < LMin) LMin = D;
			if (D > LMax) LMax = D;
		}
		double RMin = MAX_dbl;
		double RMax = -MAX_dbl;
		for (const FVector& V : RightVertsWS)
		{
			const double D = FVector::DotProduct(Axis, V);
			if (D < RMin) RMin = D;
			if (D > RMax) RMax = D;
		}
		return LMax >= RMin && RMax >= LMin;
	};

	for (const FNRawMeshLoop& Face : LeftMesh.FaceLoops)
	{
		if (Face.Indices.Num() < 3) continue;
		const FVector& V0 = LeftVertsWS[Face.Indices[0]];
		const FVector& V1 = LeftVertsWS[Face.Indices[1]];
		const FVector& V2 = LeftVertsWS[Face.Indices[2]];
		const FVector WorldAxis = FVector::CrossProduct(V1 - V0, V2 - V0);
		if (!IntervalsOverlap(WorldAxis))
		{
			return false;
		}
	}

	for (const FNRawMeshLoop& Face : RightMesh.FaceLoops)
	{
		if (Face.Indices.Num() < 3) continue;
		const FVector& V0 = RightVertsWS[Face.Indices[0]];
		const FVector& V1 = RightVertsWS[Face.Indices[1]];
		const FVector& V2 = RightVertsWS[Face.Indices[2]];
		const FVector WorldAxis = FVector::CrossProduct(V1 - V0, V2 - V0);
		if (!IntervalsOverlap(WorldAxis))
		{
			return false;
		}
	}

	// No face normal separated — hand off to the baked tri-tri sweep (same reasoning as the transform path).
	return DoesIntersectTrianglesBaked(LeftMesh, RightMesh);
}

// #SONARQUBE-ENABLE