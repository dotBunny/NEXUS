// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Types/NRawMeshFactory.h"

#include "Chaos/TriangleMeshImplicitObject.h"
#include "PhysicsEngine/BodySetup.h"


namespace NEXUS::Core::RawMeshFactory
{
	constexpr int32 SphereSegments = 16;
	constexpr int32 SphereRings = 8;
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
	static const int32 Tris[12][3] = {
		{ 0, 2, 1 }, { 0, 3, 2 },
		{ 4, 5, 6 }, { 4, 6, 7 },
		{ 0, 1, 5 }, { 0, 5, 4 },
		{ 1, 2, 6 }, { 1, 6, 5 },
		{ 2, 3, 7 }, { 2, 7, 6 },
		{ 3, 0, 4 }, { 3, 4, 7 },
	};

	Mesh.Loops.Reserve(12);
	for (int32 i = 0; i < 12; ++i)
	{
		Mesh.Loops.Add(FNRawMeshLoop(Tris[i][0], Tris[i][1], Tris[i][2]));
	}
	
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
			Mesh.Loops.Add(FNRawMeshLoop(A, B, C));
			Mesh.Loops.Add(FNRawMeshLoop(A, C, D));
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

	// Top cap fan.
	for (int32 s = 0; s < Segments; ++s)
	{
		const int32 A = 1 + s;
		const int32 B = 1 + (s + 1) % Segments;
		Mesh.Loops.Add(FNRawMeshLoop(0, A, B));
	}
	// Quad band between every adjacent ring (includes the cylindrical band).
	for (int32 r = 0; r < TotalRings - 1; ++r)
	{
		const int32 RowStart = 1 + r * Segments;
		const int32 NextRowStart = RowStart + Segments;
		for (int32 s = 0; s < Segments; ++s)
		{
			const int32 A = RowStart + s;
			const int32 B = RowStart + (s + 1) % Segments;
			const int32 C = NextRowStart + (s + 1) % Segments;
			const int32 D = NextRowStart + s;
			Mesh.Loops.Add(FNRawMeshLoop(A, B, C));
			Mesh.Loops.Add(FNRawMeshLoop(A, C, D));
		}
	}
	// Bottom cap fan.
	constexpr int32 LastRowStart = 1 + (TotalRings - 1) * Segments;
	for (int32 s = 0; s < Segments; ++s)
	{
		const int32 A = LastRowStart + s;
		const int32 B = LastRowStart + (s + 1) % Segments;
		Mesh.Loops.Add(FNRawMeshLoop(A, BottomPole, B));
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
	Mesh.Validate();
	Mesh.CalculateCenterAndBounds();
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
	if (ConvexHull.VertexData.Num() == 0 || ConvexHull.IndexData.Num() < 3) return false;

	FNRawMesh Mesh;
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
	Mesh.Validate();
	Mesh.CalculateCenterAndBounds();
	OutMesh = MoveTemp(Mesh);
	return true;
}

bool FNRawMeshFactory::FromStaticMesh(const UStaticMesh* StaticMesh, FNRawMesh& OutMesh)
{
	if (!StaticMesh || !StaticMesh->GetRenderData() || StaticMesh->GetRenderData()->LODResources.Num() == 0)
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
	Mesh.Validate();
	Mesh.CalculateCenterAndBounds();
	OutMesh = MoveTemp(Mesh);
	return true;
}
