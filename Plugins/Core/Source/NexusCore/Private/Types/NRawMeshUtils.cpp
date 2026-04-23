// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMeshUtils.h"

#include "EngineUtils.h"
#include "NCoreMinimal.h"
#include "ParticleHelper.h"
#include "StaticMeshCompiler.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Developer/NMethodScopeTimer.h"
#include "Math/NBoundsUtils.h"
#include "Math/NTriangleUtils.h"
#include "Math/NVectorUtils.h"
#include "PhysicsEngine/BodySetup.h"
#include "Types/NRawMeshFactory.h"

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

void FNRawMeshUtils::GetWorldSimpleCollisionMeshes(const UWorld* World, const TArray<FBoxSphereBounds>& ContainingBounds, 
	TFunctionRef<bool(const AActor*)> ActorFilter, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms)
{
#if WITH_EDITOR
	// Static meshes compile async in editor; force any pending compiles to finish so every BodySetup is populated.
	TArray<UStaticMesh*> Pending;
	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		if (It->IsCompiling())
		{
			Pending.Add(*It);
		}
	}
	if (Pending.Num() > 0)
	{
		FStaticMeshCompilingManager::Get().FinishCompilation(Pending);
	}
#endif // WITH_EDITOR

	for (TActorIterator<AActor> WorldActorIterator(World); WorldActorIterator; ++WorldActorIterator)
	{
		AActor* Actor = *WorldActorIterator;
		if (!Actor || Actor->IsEditorOnly()) continue;
		if (!ActorFilter(Actor)) continue;

		// Actor-bounds containment filter: include only when the actor's bounds fit inside at least one supplied bounds.
		if (!ContainingBounds.IsEmpty())
		{
			FVector ActorOrigin;
			FVector ActorExtent;
			Actor->GetActorBounds(true, ActorOrigin, ActorExtent, true);
			const FBoxSphereBounds ActorBounds(ActorOrigin, ActorExtent, ActorExtent.Size());
			bool bWithinAny = false;
			for (const FBoxSphereBounds& Bounds : ContainingBounds)
			{
				if (FNBoundsUtils::IsBoundsContainedInBounds(ActorBounds, Bounds))
				{
					bWithinAny = true;
					break;
				}
			}
			if (!bWithinAny) continue;
		}

		TInlineComponentArray<UPrimitiveComponent*> ActorPrimitives(Actor);
		for (UPrimitiveComponent* ActorPrimitive : ActorPrimitives)
		{
			if (!ActorPrimitive || !ActorPrimitive->IsRegistered()) continue;
			if (IsLandscapePrimitive(ActorPrimitive)) continue;

			UBodySetup* Body = ActorPrimitive->GetBodySetup();
			if (!Body) continue;

			const ECollisionTraceFlag Trace = Body->GetCollisionTraceFlag();
			const bool bUseComplexAsSimple = (Trace == CTF_UseComplexAsSimple);

			// Instanced variants share one BodySetup across N instances — emit per instance.
			if (const UInstancedStaticMeshComponent* InstanceStaticMesh = Cast<UInstancedStaticMeshComponent>(ActorPrimitive))
			{
				const int32 InstanceCount = InstanceStaticMesh->GetInstanceCount();
				for (int32 i = 0; i < InstanceCount; ++i)
				{
					FTransform InstanceWorld;
					InstanceStaticMesh->GetInstanceTransform(i, InstanceWorld,true);

					if (bUseComplexAsSimple)
					{
						FNRawMesh InstanceMesh;
						if (FNRawMeshFactory::FromStaticMesh(InstanceStaticMesh->GetStaticMesh(), InstanceMesh))
						{
							OutMeshes.Add(MoveTemp(InstanceMesh));
							OutTransforms.Add(MoveTemp(InstanceWorld));
						}
					}
					else
					{
						AppendChaosAggregateGeometry(Body->AggGeom, InstanceWorld,OutMeshes, OutTransforms);
					}
				}
				continue;
			}

			const FTransform CompToWorld = ActorPrimitive->GetComponentTransform();

			if (bUseComplexAsSimple)
			{
				// Route 1 — StaticMeshComponent: read source triangles from render data.
				if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ActorPrimitive))
				{
					FNRawMesh InstanceMesh;
					if (FNRawMeshFactory::FromStaticMesh(StaticMeshComponent->GetStaticMesh(), InstanceMesh))
					{
						OutMeshes.Add(MoveTemp(InstanceMesh));
						OutTransforms.Add(CompToWorld);
						continue;
					}
				}

				// Route 2 — everything else (procedural meshes, destructibles, etc):
				// ensure physics meshes are built and read the Chaos tri mesh directly.
				if (!Body->bCreatedPhysicsMeshes)
				{
					Body->CreatePhysicsMeshes();
				}
				
				FNRawMeshFactory::FromChaosBodySetup(Body, CompToWorld, OutMeshes, OutTransforms);
				continue;
			}

			AppendChaosAggregateGeometry(Body->AggGeom, CompToWorld,OutMeshes, OutTransforms);
		}
	}
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

void FNRawMeshUtils::AppendChaosAggregateGeometry(const FKAggregateGeom& Agg, const FTransform& BaseToWorld, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms)
{
	for (const FKConvexElem& ConvexHull : Agg.ConvexElems)
	{
		FNRawMesh WorkingMesh;
		if (FNRawMeshFactory::FromChaosConvexHull(ConvexHull, WorkingMesh))
		{
			OutMeshes.Add(MoveTemp(WorkingMesh));
			OutTransforms.Add(ConvexHull.GetTransform() * BaseToWorld);
		}
	}
	for (const FKBoxElem& Box : Agg.BoxElems)
	{
		FNRawMesh WorkingMesh;
		FTransform WorkingTransform;
		if (FNRawMeshFactory::FromChaosBox(Box, BaseToWorld, WorkingMesh, WorkingTransform))
		{
			OutMeshes.Add(MoveTemp(WorkingMesh));
			OutTransforms.Add(MoveTemp(WorkingTransform));
		}
	}
	for (const FKSphereElem& Sphere : Agg.SphereElems)
	{
		FNRawMesh WorkingMesh;
		FTransform WorkingTransform;
		if (FNRawMeshFactory::FromChaosSphere(Sphere, BaseToWorld, WorkingMesh, WorkingTransform))
		{
			OutMeshes.Add(MoveTemp(WorkingMesh));
			OutTransforms.Add(MoveTemp(WorkingTransform));
		}
	}
	for (const FKSphylElem& Sphyl : Agg.SphylElems)
	{
		FNRawMesh WorkingMesh;
		FTransform WorkingTransform;
		if (FNRawMeshFactory::FromChaosSphyl(Sphyl, BaseToWorld, WorkingMesh, WorkingTransform))
		{
			OutMeshes.Add(MoveTemp(WorkingMesh));
			OutTransforms.Add(MoveTemp(WorkingTransform));
		}
	}
}

bool FNRawMeshUtils::IsLandscapePrimitive(const UPrimitiveComponent* Prim)
{
	// Avoids taking a hard dependency on the Landscape module.
	return Prim->GetClass()->GetName().StartsWith(TEXT("Landscape"));
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
