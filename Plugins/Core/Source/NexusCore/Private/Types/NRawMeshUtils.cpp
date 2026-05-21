// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMeshUtils.h"

#include "NCoreMinimal.h"
#include "Chaos/Convex.h"
#include "Math/NTriangleUtils.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMeshFactory.h"

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
	
	return DoesIntersectTriangles(LeftMesh, LeftOrigin, LeftRotation, RightMesh, RightOrigin, RightRotation);
}

float FNRawMeshUtils::GetIntersectDepth(
	const FNRawMesh& LeftMesh, const FVector& LeftOrigin, const FRotator& LeftRotation,
	const FNRawMesh& RightMesh, const FVector& RightOrigin, const FRotator& RightRotation)
{
	// Rotated AABB early-out, mirroring DoesIntersect.
	if (LeftMesh.HasBounds() && RightMesh.HasBounds())
	{
		const FBox LeftWorldBounds = LeftMesh.Bounds.TransformBy(FTransform(LeftRotation, LeftOrigin));
		const FBox RightWorldBounds = RightMesh.Bounds.TransformBy(FTransform(RightRotation, RightOrigin));
		if (!LeftWorldBounds.Intersect(RightWorldBounds))
		{
			return -1.0f;
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

	// Right vertices, expressed in Left's local space, tested against Left's hull.
	for (const FVector& V : RightMesh.Vertices)
	{
		const FVector WorldPos = RightQuat.RotateVector(V) + RightOrigin;
		const FVector LeftLocal = LeftInvQuat.RotateVector(WorldPos - LeftOrigin);
		const float Depth = ComputePointDepthInside(LeftMesh, LeftLocal);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
		}
	}

	// Left vertices, expressed in Right's local space, tested against Right's hull.
	for (const FVector& V : LeftMesh.Vertices)
	{
		const FVector WorldPos = LeftQuat.RotateVector(V) + LeftOrigin;
		const FVector RightLocal = RightInvQuat.RotateVector(WorldPos - RightOrigin);
		const float Depth = ComputePointDepthInside(RightMesh, RightLocal);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
		}
	}

	return MaxDepth;
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
		if (DebugActor == nullptr) return MoveTemp(DebugActors);
		
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
	return MoveTemp(DebugActors);
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
		return MoveTemp(Result);
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
		return MoveTemp(Result);
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

	return MoveTemp(Result);
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
	double MinDist = TNumericLimits<double>::Max();
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
	return (MinDist == TNumericLimits<double>::Max()) ? 0.0f : static_cast<float>(MinDist);
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

	float MinFaceDist = TNumericLimits<float>::Max();
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
	return (MinFaceDist == TNumericLimits<float>::Max()) ? 0.0f : MinFaceDist;
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

	// Triangle-triangle tests only catch surface crossings. If one mesh is entirely contained
	// within the other, no triangles will intersect but the meshes still overlap. Check
	// containment by testing one vertex from each mesh against the other mesh; for a closed
	// manifold (convex or not), a single sample is enough — either every vertex is enclosed or
	// none is, because any surface intersection would have been caught above.
	const FQuat LeftInvQuat = LeftRotation.Quaternion().Inverse();
	const FQuat RightInvQuat = RightRotation.Quaternion().Inverse();

	const FVector LeftSample = LeftInvQuat.RotateVector(LeftVerticesWorld[0] - RightOrigin);
	if (IsRelativePointInside(RightMesh, LeftSample))
	{
		return true;
	}

	const FVector RightSample = RightInvQuat.RotateVector(RightVerticesWorld[0] - LeftOrigin);
	if (IsRelativePointInside(LeftMesh, RightSample))
	{
		return true;
	}

	return false;
}
