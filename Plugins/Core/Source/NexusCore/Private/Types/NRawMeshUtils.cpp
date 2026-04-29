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
	if (OtherMesh.Vertices.Num() == 0 || OtherMesh.Loops.Num() == 0)
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

	// A merged mesh is no longer a single Chaos-cooked body; force Validate() to re-evaluate convexity and tri-ness.
	BaseMesh.bIsChaosGenerated = false;

	BaseMesh.CalculateCenterAndBounds();
	BaseMesh.Validate();
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

	if (LeftMesh.Loops.Num() == 0 || RightMesh.Loops.Num() == 0)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("No loops were found in the provided FNRawMeshes, unable to determine if there is any intersection."))
		return false;
	}
	if (LeftMesh.HasNonTris() || RightMesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Error, TEXT("One or both of the provided FNRawMeshes has non-triangle based geometry (NGons), this is not supported for intersection checks."));
		return false;
	}
	
	return DoesIntersectTriangles(LeftMesh, LeftOrigin, LeftRotation, RightMesh, RightOrigin, RightRotation);
}

TArray<ANDebugActor*> FNRawMeshUtils::CreateRawMeshVisualizers(UWorld* World, TArray<FNRawMesh>& Meshes, const TArray<FTransform>& Transforms,  UMaterialInterface* MaterialInterface, bool bSingleActor, bool bProcessMeshes)
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
		UE_LOG(LogNexusCore, Warning, TEXT("Converting mesh to ConvexHull that is already a convex hull. This duplicates the mesh!!!"))
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

	Result.Loops.Reserve(LoopCount);
	for (int32 i = 0; i < LoopCount; i++)
	{
		Result.Loops.Add(FNRawMeshLoop(OutFaceIndices[i]));
	}

	Result.Center = CenterCalc / VerticesCount;
	Result.Bounds = BoundingBox;

	// Trust the Chaos output: faces may be n-gons but are convex by construction.
	Result.bIsChaosGenerated = true;
	Result.bIsConvex = true;
	Result.bHasBounds = true;
	Result.bHasNonTris = Result.CheckNonTris();

	if (Result.bHasNonTris)
	{
		Result.ConvertToTriangles();
	}

	return MoveTemp(Result);
}

bool FNRawMeshUtils::IsRelativePointInside(const FNRawMesh& Mesh, const FVector& RelativePoint)
{
	if (!Mesh.IsConvex() || Mesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The FNRawMesh is either not convex or has non-triangles; unable to determine IsRelativePointInside; returning false."));
		return false;
	}
	
	for (const FNRawMeshLoop& Loop : Mesh.Loops)
	{
		const FVector& V0 = Mesh.Vertices[Loop.Indices[0]];
		const FVector& V1 = Mesh.Vertices[Loop.Indices[1]];
		const FVector& V2 = Mesh.Vertices[Loop.Indices[2]];
		
		const FVector Normal = FVector::CrossProduct(V1 - V0, V2 - V0);
		const float CenterSide = FVector::DotProduct(Normal, Mesh.Center - V0);
		const float PointSide  = FVector::DotProduct(Normal, RelativePoint - V0);
		
		if (CenterSide * PointSide < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool FNRawMeshUtils::AnyRelativePointsInside(const FNRawMesh& Mesh, const TArray<FVector>& RelativePoints)
{
	if (!Mesh.IsConvex() || Mesh.HasNonTris())
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The FNRawMesh is either not convex or has non-triangles; unable to determine AnyRelativePointsInside; returning false."));
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
	
	// Check every triangle against every other triangle
	const int32 LeftLoopCount = LeftMesh.Loops.Num();
	const int32 RightLoopCount = RightMesh.Loops.Num();

	for (int32 i = 0; i < LeftLoopCount; ++i)
	{
		const FVector& V0 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[0]];
		const FVector& V1 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[1]];
		const FVector& V2 = LeftVerticesWorld[LeftMesh.Loops[i].Indices[2]];

		for (int32 j = 0; j < RightLoopCount; ++j)
		{
			const FVector& U0 = RightVerticesWorld[RightMesh.Loops[j].Indices[0]];
			const FVector& U1 = RightVerticesWorld[RightMesh.Loops[j].Indices[1]];
			const FVector& U2 = RightVerticesWorld[RightMesh.Loops[j].Indices[2]];

			if (FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2))
			{
				return true;
			}
		}
	}

	// Triangle-triangle tests only catch surface crossings. If one convex mesh is entirely
	// contained within the other, no triangles will intersect but the meshes still overlap.
	// Check containment by testing one vertex from each mesh against the other mesh.
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
