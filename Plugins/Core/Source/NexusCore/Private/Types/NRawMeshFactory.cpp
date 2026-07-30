// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMeshFactory.h"

#include "Chaos/Convex.h"
#include "Chaos/TriangleMeshImplicitObject.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Developer/NDeveloperUtils.h"
#include "Engine/StaticMesh.h"
#include "Math/NBoundsUtils.h"
#include "PhysicsEngine/BodySetup.h"

namespace NEXUS::Core::RawMeshFactory
{
	constexpr int32 SphereSegments = 16;
	constexpr int32 SphereRings = 8;
}

void FNRawMeshFactory::FromActorsInBounds(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& ContainingBounds, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms)
{
	// Static meshes compile async in editor; force any pending compiles to finish so every BodySetup is populated.
	FNDeveloperUtils::WaitForStaticMeshCompilation();

	for (AActor* Actor : Actors)
	{
		if (!IsValid(Actor)) continue;

		// Actor-bounds overlap filter: include when the actor's bounds intersect at least one supplied bounds.
		if (!ContainingBounds.IsEmpty())
		{
			FVector ActorOrigin;
			FVector ActorExtent;
			Actor->GetActorBounds(true, ActorOrigin, ActorExtent, true);
			const FBox ActorBox(ActorOrigin - ActorExtent, ActorOrigin + ActorExtent);
			bool bOverlapsAny = false;
			for (const FBoxSphereBounds& Bounds : ContainingBounds)
			{
				if (ActorBox.Intersect(Bounds.GetBox()))
				{
					bOverlapsAny = true;
					break;
				}
			}
			if (!bOverlapsAny) continue;
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
				const UStaticMesh* SourceMesh = InstanceStaticMesh->GetStaticMesh();
				const bool bUseRenderData = bUseComplexAsSimple && IsStaticMeshCPUReadable(SourceMesh);

				// The render-data route needs no physics meshes. Every other route does: the simple path so
				// AppendChaosAggregateGeometry reads polygonal ConvexElem face data instead of falling back to the
				// triangulated IndexData, and the complex-as-simple route-2 fallback so TriMeshGeometries is populated.
				if (!bUseRenderData && !Body->bCreatedPhysicsMeshes)
				{
					Body->CreatePhysicsMeshes();
				}

				const int32 InstanceCount = InstanceStaticMesh->GetInstanceCount();
				for (int32 i = 0; i < InstanceCount; ++i)
				{
					FTransform InstanceWorld;
					InstanceStaticMesh->GetInstanceTransform(i, InstanceWorld,true);

					if (!bUseComplexAsSimple)
					{
						AppendChaosAggregateGeometry(Body->AggGeom, InstanceWorld,OutMeshes, OutTransforms);
					}
					else if (bUseRenderData)
					{
						FNRawMesh InstanceMesh;
						if (FromStaticMesh(SourceMesh, InstanceMesh))
						{
							OutMeshes.Add(MoveTemp(InstanceMesh));
							OutTransforms.Add(MoveTemp(InstanceWorld));
						}
					}
					else
					{
						// Render data isn't CPU-readable (cooked build without CPU access) — fall back to the
						// cooked Chaos tri mesh, placed at this instance's world transform.
						FromChaosBodySetup(Body, InstanceWorld, OutMeshes, OutTransforms);
					}
				}
				continue;
			}

			const FTransform CompToWorld = ActorPrimitive->GetComponentTransform();

			if (bUseComplexAsSimple)
			{
				// Route 1 — StaticMeshComponent: read source triangles from render data. Editor / CPU-accessible
				// meshes only; cooked builds free the CPU buffer copies, so guard before reading them.
				if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ActorPrimitive))
				{
					const UStaticMesh* SourceMesh = StaticMeshComponent->GetStaticMesh();
					if (IsStaticMeshCPUReadable(SourceMesh))
					{
						FNRawMesh InstanceMesh;
						if (FromStaticMesh(SourceMesh, InstanceMesh))
						{
							OutMeshes.Add(MoveTemp(InstanceMesh));
							OutTransforms.Add(CompToWorld);
							continue;
						}
					}
				}

				// Route 2 — everything else (procedural meshes, destructibles, etc), and the runtime-safe fallback
				// when render data isn't CPU-readable: ensure physics meshes are built and read the Chaos tri mesh directly.
				if (!Body->bCreatedPhysicsMeshes)
				{
					Body->CreatePhysicsMeshes();
				}

				FromChaosBodySetup(Body, CompToWorld, OutMeshes, OutTransforms);
				continue;
			}

			// Same reason as the instanced path above — populate ConvexElem.ChaosConvex so
			// FromChaosConvexHull can pull polygonal face data instead of falling back to triangles.
			if (!Body->bCreatedPhysicsMeshes)
			{
				Body->CreatePhysicsMeshes();
			}

			AppendChaosAggregateGeometry(Body->AggGeom, CompToWorld,OutMeshes, OutTransforms);
		}
	}
}

bool FNRawMeshFactory::FromChaosBox(const FKBoxElem& Box, const FTransform& CompToWorld, FNRawMesh& OutMesh, FTransform& OutTransform)
{
	const double HX = Box.X * 0.5;
	const double HY = Box.Y * 0.5;
	const double HZ = Box.Z * 0.5;

	FNRawMesh Mesh;
	Mesh.Vertices = {
					{ -HX, -HY, -HZ }, { +HX, -HY, -HZ }, { +HX, +HY, -HZ }, { -HX, +HY, -HZ },
					{ -HX, -HY, +HZ }, { +HX, -HY, +HZ }, { +HX, +HY, +HZ }, { -HX, +HY, +HZ },
				};
	static const TStaticArray<FIntVector, 12> Tris{
		FIntVector{ 0, 2, 1 }, FIntVector{ 0, 3, 2 },
		FIntVector{ 4, 5, 6 }, FIntVector{ 4, 6, 7 },
		FIntVector{ 0, 1, 5 }, FIntVector{ 0, 5, 4 },
		FIntVector{ 1, 2, 6 }, FIntVector{ 1, 6, 5 },
		FIntVector{ 2, 3, 7 }, FIntVector{ 2, 7, 6 },
		FIntVector{ 3, 0, 4 }, FIntVector{ 3, 4, 7 },
	};

	Mesh.Loops.Reserve(12);
	for (const FIntVector& Tri : Tris)
	{
		Mesh.Loops.Add(FNRawMeshLoop(Tri.X, Tri.Y, Tri.Z));
	}

	// Polygonal face description for CheckConvex — six outward-wound quads matching the fan splits above.
	Mesh.FaceLoops.Reserve(6);
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 3, 2, 1)); // -Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(4, 5, 6, 7)); // +Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 1, 5, 4)); // -Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(1, 2, 6, 5)); // +X
	Mesh.FaceLoops.Add(FNRawMeshLoop(2, 3, 7, 6)); // +Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(3, 0, 4, 7)); // -X

	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();


	const FTransform ElemLocal(Box.Rotation, Box.Center);
	OutMesh = MoveTemp(Mesh);
	OutTransform = ElemLocal * CompToWorld;
	return true;
}

bool FNRawMeshFactory::FromChaosSphere(const FKSphereElem& Sphere, const FTransform& CompToWorld, FNRawMesh& OutMesh, FTransform& OutTransform)
{
	const double R = Sphere.Radius;

	FNRawMesh Mesh;
	Mesh.Vertices.Reserve(2 + (NEXUS::Core::RawMeshFactory::SphereRings - 1) * NEXUS::Core::RawMeshFactory::SphereSegments);
	Mesh.Vertices.Add(FVector(0.0, 0.0, R));
	for (int32 r = 1; r < NEXUS::Core::RawMeshFactory::SphereRings; ++r)
	{
		const double Phi = PI * r / NEXUS::Core::RawMeshFactory::SphereRings;
		const double Z = R * FMath::Cos(Phi);
		const double RingRadius = R * FMath::Sin(Phi);
		for (int32 s = 0; s < NEXUS::Core::RawMeshFactory::SphereSegments; ++s)
		{
			const double Theta = 2.0 * PI * s / NEXUS::Core::RawMeshFactory::SphereSegments;
			Mesh.Vertices.Add(FVector(RingRadius * FMath::Cos(Theta), RingRadius * FMath::Sin(Theta), Z));
		}
	}
	Mesh.Vertices.Add(FVector(0.0, 0.0, -R));
	const int32 BottomPole = Mesh.Vertices.Num() - 1;

	Mesh.Loops.Reserve(NEXUS::Core::RawMeshFactory::SphereSegments * 2 + (NEXUS::Core::RawMeshFactory::SphereRings - 2) * NEXUS::Core::RawMeshFactory::SphereSegments * 2);
	for (int32 s = 0; s < NEXUS::Core::RawMeshFactory::SphereSegments; ++s)
	{
		const int32 A = 1 + s;
		const int32 B = 1 + (s + 1) % NEXUS::Core::RawMeshFactory::SphereSegments;
		Mesh.Loops.Add(FNRawMeshLoop(0, A, B));
	}
	// Band quads wind outward as (A, D, C, B) when viewed from outside the sphere — A/B on the upper ring,
	// C/D on the lower ring. Fan-from-A emits the two outward-CCW triangles below.
	for (int32 r = 0; r < NEXUS::Core::RawMeshFactory::SphereRings - 2; ++r)
	{
		const int32 RowStart = 1 + r * NEXUS::Core::RawMeshFactory::SphereSegments;
		const int32 NextRowStart = RowStart + NEXUS::Core::RawMeshFactory::SphereSegments;
		for (int32 s = 0; s < NEXUS::Core::RawMeshFactory::SphereSegments; ++s)
		{
			const int32 A = RowStart + s;
			const int32 B = RowStart + (s + 1) % NEXUS::Core::RawMeshFactory::SphereSegments;
			const int32 C = NextRowStart + (s + 1) % NEXUS::Core::RawMeshFactory::SphereSegments;
			const int32 D = NextRowStart + s;
			Mesh.Loops.Add(FNRawMeshLoop(A, D, C));
			Mesh.Loops.Add(FNRawMeshLoop(A, C, B));
		}
	}
	constexpr int32 LastRowStart = 1 + (NEXUS::Core::RawMeshFactory::SphereRings - 2) * NEXUS::Core::RawMeshFactory::SphereSegments;
	for (int32 s = 0; s < NEXUS::Core::RawMeshFactory::SphereSegments; ++s)
	{
		const int32 A = LastRowStart + s;
		const int32 B = LastRowStart + (s + 1) % NEXUS::Core::RawMeshFactory::SphereSegments;
		Mesh.Loops.Add(FNRawMeshLoop(A, BottomPole, B));
	}
	Mesh.Validate();

	const FTransform ElemLocal(FQuat::Identity, Sphere.Center);
	OutMesh = MoveTemp(Mesh);
	OutTransform = ElemLocal * CompToWorld;
	return true;
}

bool FNRawMeshFactory::FromChaosSphyl(const FKSphylElem& Sphyl, const FTransform& CompToWorld, FNRawMesh& OutMesh, FTransform& OutTransform)
{
	constexpr int32 Segments = NEXUS::Core::RawMeshFactory::SphereSegments;
	constexpr int32 HemiRings = 4;
	const double R = Sphyl.Radius;
	const double HalfLength = Sphyl.Length * 0.5;

	FNRawMesh Mesh;
	Mesh.Vertices.Add(FVector(0.0, 0.0, HalfLength + R));

	// Top hemisphere down to the top equator (Z = HalfLength).
	for (int32 r = 1; r <= HemiRings; ++r)
	{
		const double Phi = (PI * 0.5) * r / HemiRings;
		const double Z = HalfLength + R * FMath::Cos(Phi);
		const double RingRadius = R * FMath::Sin(Phi);
		for (int32 s = 0; s < Segments; ++s)
		{
			const double Theta = 2.0 * PI * s / Segments;
			Mesh.Vertices.Add(FVector(RingRadius * FMath::Cos(Theta), RingRadius * FMath::Sin(Theta), Z));
		}
	}

	// Cylinder-bottom ring (Z = -HalfLength).
	for (int32 s = 0; s < Segments; ++s)
	{
		const double Theta = 2.0 * PI * s / Segments;
		Mesh.Vertices.Add(FVector(R * FMath::Cos(Theta), R * FMath::Sin(Theta), -HalfLength));
	}

	// Bottom hemisphere down to just above the pole.
	for (int32 r = 1; r < HemiRings; ++r)
	{
		const double Phi = (PI * 0.5) * (HemiRings - r) / HemiRings;
		const double Z = -HalfLength - R * FMath::Cos(Phi);
		const double RingRadius = R * FMath::Sin(Phi);
		for (int32 s = 0; s < Segments; ++s)
		{
			const double Theta = 2.0 * PI * s / Segments;
			Mesh.Vertices.Add(FVector(RingRadius * FMath::Cos(Theta), RingRadius * FMath::Sin(Theta), Z));
		}
	}
	Mesh.Vertices.Add(FVector(0.0, 0.0, -HalfLength - R));

	constexpr int32 TotalRings = 2 * HemiRings;
	constexpr int32 BottomPole = 1 + TotalRings * Segments;

	// Top cap fan — pole tris are intrinsically triangular, so FaceLoops mirrors Loops here.
	for (int32 s = 0; s < Segments; ++s)
	{
		const int32 A = 1 + s;
		const int32 B = 1 + (s + 1) % Segments;
		Mesh.Loops.Add(FNRawMeshLoop(0, A, B));
		Mesh.FaceLoops.Add(FNRawMeshLoop(0, A, B));
	}
	// Quad band between every adjacent ring (includes the cylindrical band). A/B on the upper ring, C/D on the lower
	// ring; the outward-CCW winding viewed from outside the sphyl is (A, D, C, B), so each quad fans into the two
	// triangles below. Hemisphere-band quads are saddle-shaped on the sphere surface and don't form a planar polygon,
	// so FaceLoops keeps them as the same two triangles as Loops. The cylindrical band (between the top equator and
	// the cylinder bottom ring) IS planar — its four vertices form a vertical rectangle — so FaceLoops emits it as a
	// single quad and CheckConvex tests one supporting plane per band segment instead of two (which would otherwise
	// drift apart after a vertex edit).
	for (int32 r = 0; r < TotalRings - 1; ++r)
	{
		const int32 RowStart = 1 + r * Segments;
		const int32 NextRowStart = RowStart + Segments;
		const bool bIsCylinderBand = (r == HemiRings - 1);
		for (int32 s = 0; s < Segments; ++s)
		{
			const int32 A = RowStart + s;
			const int32 B = RowStart + (s + 1) % Segments;
			const int32 C = NextRowStart + (s + 1) % Segments;
			const int32 D = NextRowStart + s;
			Mesh.Loops.Add(FNRawMeshLoop(A, D, C));
			Mesh.Loops.Add(FNRawMeshLoop(A, C, B));
			if (bIsCylinderBand)
			{
				Mesh.FaceLoops.Add(FNRawMeshLoop(A, D, C, B));
			}
			else
			{
				Mesh.FaceLoops.Add(FNRawMeshLoop(A, D, C));
				Mesh.FaceLoops.Add(FNRawMeshLoop(A, C, B));
			}
		}
	}
	// Bottom cap fan — pole tris, FaceLoops mirrors Loops.
	constexpr int32 LastRowStart = 1 + (TotalRings - 1) * Segments;
	for (int32 s = 0; s < Segments; ++s)
	{
		const int32 A = LastRowStart + s;
		const int32 B = LastRowStart + (s + 1) % Segments;
		Mesh.Loops.Add(FNRawMeshLoop(A, BottomPole, B));
		Mesh.FaceLoops.Add(FNRawMeshLoop(A, BottomPole, B));
	}

	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	const FTransform ElemLocal(Sphyl.Rotation, Sphyl.Center);
	OutMesh = MoveTemp(Mesh);
	OutTransform = ElemLocal * CompToWorld;
	return true;
}

bool FNRawMeshFactory::FromChaosTriMeshes(const Chaos::FTriangleMeshImplicitObjectPtr& TriMesh, FNRawMesh& OutMesh)
{
	if (!TriMesh.IsValid()) return false;

	const auto& Particles = TriMesh->Particles();
	const Chaos::FTrimeshIndexBuffer& IdxBuffer = TriMesh->Elements();

	const int32 NumVerts = static_cast<int32>(Particles.Size());
	const int32 NumTris = IdxBuffer.GetNumTriangles();
	if (NumVerts == 0 || NumTris == 0) return false;

	FNRawMesh Mesh;
	Mesh.Vertices.Reserve(NumVerts);
	for (int32 i = 0; i < NumVerts; ++i)
	{
		const Chaos::TVec3<Chaos::FRealSingle> P = Particles.GetX(i);
		Mesh.Vertices.Add(FVector(P.X, P.Y, P.Z));
	}

	Mesh.Loops.Reserve(NumTris);
	if (IdxBuffer.RequiresLargeIndices())
	{
		for (const Chaos::TVec3<int32>& Tri : IdxBuffer.GetLargeIndexBuffer())
		{
			Mesh.Loops.Add(FNRawMeshLoop(Tri[0], Tri[1], Tri[2]));
		}
	}
	else
	{
		for (const Chaos::TVec3<uint16>& Tri : IdxBuffer.GetSmallIndexBuffer())
		{
			Mesh.Loops.Add(FNRawMeshLoop(Tri[0], Tri[1],Tri[2]));
		}
	}
	// Recover polygonal faces from the triangle list so CheckConvex sees merged n-gons rather than fan-triangulated
	// coplanar groups. For sculpted / organic source meshes this is a no-op; for CSG-style box aggregates it recovers quads.
	Mesh.CalculateCenterAndBounds();
	Mesh.CalculateFaceLoops();
	Mesh.Validate();
	OutMesh = MoveTemp(Mesh);

	return true;
}

bool FNRawMeshFactory::FromChaosBodySetup(const UBodySetup* Body, const FTransform& ToWorld, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms)
{
	for (const Chaos::FTriangleMeshImplicitObjectPtr& TriMeshPtr : Body->TriMeshGeometries)
	{
		FNRawMesh Mesh;
		if (FromChaosTriMeshes(TriMeshPtr, Mesh))
		{
			OutMeshes.Add(MoveTemp(Mesh));
			OutTransforms.Add(ToWorld);
		}
	}
	return true;
}

bool FNRawMeshFactory::FromChaosConvexHull(const FKConvexElem& ConvexHull, FNRawMesh& OutMesh)
{
	if (ConvexHull.VertexData.IsEmpty() || ConvexHull.IndexData.Num() < 3) return false;

	FNRawMesh Mesh;

	// When the FKConvexElem carries a live Chaos::FConvex (the common case for cooked physics bodies), prefer its
	// polygonal face data over the triangulated IndexData — that's the same n-gon description Chaos used to build the
	// hull, with no coplanar-merge work needed on our side. The Chaos convex has its own filtered vertex array (a
	// deduplicated subset of FKConvexElem.VertexData), so we take vertices from there too to keep face indices valid.
	// Fall back to the IndexData triangulation when no Chaos convex is attached (e.g. hand-built test inputs).
	const Chaos::FConvexPtr& ChaosConvex = ConvexHull.GetChaosConvexMesh();
	const bool bHasChaosFaceData = ChaosConvex.IsValid()
		&& ChaosConvex->NumPlanes() > 0
		&& ChaosConvex->NumPlaneVertices(0) >= 3;

	if (bHasChaosFaceData)
	{
		const int32 NumVerts = ChaosConvex->NumVertices();
		Mesh.Vertices.Reserve(NumVerts);
		for (int32 i = 0; i < NumVerts; ++i)
		{
			const Chaos::FConvex::FVec3Type& V = ChaosConvex->GetVertex(i);
			Mesh.Vertices.Add(FVector(V[0], V[1], V[2]));
		}

		const int32 NumPlanes = ChaosConvex->NumPlanes();
		Mesh.FaceLoops.Reserve(NumPlanes);
		for (int32 p = 0; p < NumPlanes; ++p)
		{
			const int32 NumPlaneVerts = ChaosConvex->NumPlaneVertices(p);
			TArray<int32> Indices;
			Indices.Reserve(NumPlaneVerts);
			for (int32 v = 0; v < NumPlaneVerts; ++v)
			{
				Indices.Add(ChaosConvex->GetPlaneVertex(p, v));
			}
			Mesh.FaceLoops.Add(FNRawMeshLoop(MoveTemp(Indices)));
		}
		Mesh.Loops = Mesh.FaceLoops;

		// Chaos hulls are convex by construction; flag both so ConvertToTriangles takes the cheap fan path and Validate
		// short-circuits on bIsChaosGenerated.
		Mesh.bIsChaosGenerated = true;
		Mesh.bIsConvex = true;
		Mesh.ConvertToTriangles();
		Mesh.bHasNonTris = false;
	}
	else
	{
		Mesh.Vertices = ConvexHull.VertexData;
		const int32 TriCount = ConvexHull.IndexData.Num() / 3;
		Mesh.Loops.Reserve(TriCount);
		for (int32 i = 0; i < TriCount; ++i)
		{
			Mesh.Loops.Add(FNRawMeshLoop(
				ConvexHull.IndexData[i * 3 + 0],
				ConvexHull.IndexData[i * 3 + 1],
				ConvexHull.IndexData[i * 3 + 2]));
		}
	}

	Mesh.Validate();
	Mesh.CalculateCenterAndBounds();
	OutMesh = MoveTemp(Mesh);
	return true;
}

bool FNRawMeshFactory::FromStaticMesh(const UStaticMesh* StaticMesh, FNRawMesh& OutMesh)
{
	if (!StaticMesh || !StaticMesh->GetRenderData() || StaticMesh->GetRenderData()->LODResources.IsEmpty())
	{
		return false;
	}

	const FStaticMeshLODResources& LOD = StaticMesh->GetRenderData()->LODResources[0];
	const int32 NumVerts = LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
	if (NumVerts == 0) return false;

	FNRawMesh Mesh;
	Mesh.Vertices.Reserve(NumVerts);
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Mesh.Vertices.Add(FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(i)));
	}

	TArray<uint32> Indices;
	LOD.IndexBuffer.GetCopy(Indices);
	const int32 TriCount = Indices.Num() / 3;
	Mesh.Loops.Reserve(TriCount);
	for (int32 i = 0; i < TriCount; ++i)
	{
		Mesh.Loops.Add(FNRawMeshLoop(
			static_cast<int32>(Indices[i * 3 + 0]),
			static_cast<int32>(Indices[i * 3 + 1]),
			static_cast<int32>(Indices[i * 3 + 2])));
	}
	// Recover polygonal faces from the static mesh's triangle list. Boxy / modular assets recover quads here, which
	// gives CheckConvex an accurate polygonal description; sculpted meshes return all triangles and the call is a no-op.
	Mesh.CalculateCenterAndBounds();
	Mesh.CalculateFaceLoops();
	Mesh.Validate();
	OutMesh = MoveTemp(Mesh);
	return true;
}

void FNRawMeshFactory::AppendChaosAggregateGeometry(const FKAggregateGeom& Agg, const FTransform& BaseToWorld, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms)
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

bool FNRawMeshFactory::IsLandscapePrimitive(const UPrimitiveComponent* Prim)
{
	// Avoids taking a hard dependency on the Landscape module.
	return Prim->GetClass()->GetName().StartsWith(TEXT("Landscape"));
}

bool FNRawMeshFactory::IsStaticMeshCPUReadable(const UStaticMesh* StaticMesh)
{
	if (StaticMesh == nullptr || StaticMesh->GetRenderData() == nullptr
		|| StaticMesh->GetRenderData()->LODResources.IsEmpty())
	{
		return false;
	}
#if WITH_EDITOR
	// The editor retains CPU-side render data regardless of the asset's CPU-access flag.
	return true;
#else
	// Cooked builds free the CPU copies of the vertex/index buffers unless the mesh opted into CPU access,
	// so reading them without this guard dereferences null buffer data. Both buffers FromStaticMesh touches
	// (positions + indices) must be readable.
	const FStaticMeshLODResources& LOD = StaticMesh->GetRenderData()->LODResources[0];
	return LOD.IndexBuffer.GetAllowCPUAccess()
		&& LOD.VertexBuffers.PositionVertexBuffer.GetAllowCPUAccess();
#endif // WITH_EDITOR
}