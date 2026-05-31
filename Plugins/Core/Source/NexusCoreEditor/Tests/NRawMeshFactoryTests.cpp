// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMeshFactory.h"
#include "Chaos/Convex.h"
#include "Chaos/TriangleMeshImplicitObject.h"
#include "Components/StaticMeshComponent.h"
#include "Developer/NTestUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Macros/NTestMacros.h"
#include "PhysicsEngine/BodySetup.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNRawMeshFactoryHarness
{
	// Mirrors the constants in NRawMeshFactory.cpp's anonymous namespace so the count formulas can be re-derived here.
	constexpr int32 SphereSegments = 16;
	constexpr int32 SphereRings = 8;
	constexpr int32 SphylHemiRings = 4;
}

N_TEST_CRITICAL(FNRawMeshFactoryTests_FromChaosBox_VertexAndLoopCounts,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosBox::VertexAndLoopCounts",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKBoxElem Box;
	Box.X = 2.0;
	Box.Y = 4.0;
	Box.Z = 6.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	const bool bResult = FNRawMeshFactory::FromChaosBox(Box, FTransform::Identity, OutMesh, OutTransform);

	CHECK_MESSAGE(TEXT("FromChaosBox should report success"), bResult);
	CHECK_EQUALS("Box should produce 8 vertices", OutMesh.Vertices.Num(), 8);
	CHECK_EQUALS("Box should produce 12 triangle loops", OutMesh.Loops.Num(), 12);
	for (int32 i = 0; i < OutMesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop should be a triangle"), OutMesh.Loops[i].IsTriangle());
	}
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosBox_FaceLoops_SixOutwardQuads,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosBox::FaceLoops_SixOutwardQuads",
	N_TEST_CONTEXT_ANYWHERE)
{
	// FaceLoops should describe the box as six quads — that's what CheckConvex walks, and it's what protects against the
	// per-triangle coplanar-drift false-negative if a downstream tool ever edits a box's vertices.
	FKBoxElem Box;
	Box.X = 2.0;
	Box.Y = 4.0;
	Box.Z = 6.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosBox(Box, FTransform::Identity, OutMesh, OutTransform);

	CHECK_EQUALS("Box should produce 6 polygonal faces", OutMesh.FaceLoops.Num(), 6);
	for (int32 i = 0; i < OutMesh.FaceLoops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every box FaceLoop should be a quad"), OutMesh.FaceLoops[i].IsQuad());
	}
	CHECK_MESSAGE(TEXT("Box with FaceLoops should report convex via the polygonal CheckConvex path"), OutMesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosBox_BoundsMatchHalfExtents,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosBox::BoundsMatchHalfExtents",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKBoxElem Box;
	Box.X = 2.0;
	Box.Y = 4.0;
	Box.Z = 6.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosBox(Box, FTransform::Identity, OutMesh, OutTransform);

	CHECK_MESSAGE(TEXT("Bounds min should equal -half-extents"), OutMesh.Bounds.Min.Equals(FVector(-1, -2, -3), 0.001));
	CHECK_MESSAGE(TEXT("Bounds max should equal +half-extents"), OutMesh.Bounds.Max.Equals(FVector(1, 2, 3), 0.001));
	CHECK_MESSAGE(TEXT("Box should be reported as convex"), OutMesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosBox_TransformComposesCenterAndRotation,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosBox::TransformComposesCenterAndRotation",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKBoxElem Box;
	Box.X = 2.0;
	Box.Y = 4.0;
	Box.Z = 6.0;
	Box.Center = FVector(10, 20, 30);
	Box.Rotation = FRotator(0.f, 90.f, 0.f);

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosBox(Box, FTransform::Identity, OutMesh, OutTransform);

	CHECK_MESSAGE(TEXT("OutTransform translation should equal Box.Center when CompToWorld is identity"),
		OutTransform.GetTranslation().Equals(FVector(10, 20, 30), 0.001));
	CHECK_MESSAGE(TEXT("OutTransform rotation should equal Box.Rotation when CompToWorld is identity"),
		OutTransform.GetRotation().Equals(FQuat(FRotator(0.f, 90.f, 0.f)), 0.001));
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosConvexHull_EmptyVertexData_False,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosConvexHull::EmptyVertexData_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKConvexElem ConvexHull;
	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromChaosConvexHull(ConvexHull, OutMesh);

	CHECK_FALSE_MESSAGE(TEXT("Convex hull with no vertex data must produce false"), bResult);
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosConvexHull_TooFewIndices_False,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosConvexHull::TooFewIndices_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKConvexElem ConvexHull;
	ConvexHull.VertexData = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	ConvexHull.IndexData = { 0, 1 };

	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromChaosConvexHull(ConvexHull, OutMesh);

	CHECK_FALSE_MESSAGE(TEXT("Convex hull with fewer than 3 indices must produce false"), bResult);
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosConvexHull_ChaosConvex_PolygonalFaceLoops,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosConvexHull::ChaosConvex_PolygonalFaceLoops",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the FKConvexElem carries a live Chaos::FConvex, FromChaosConvexHull should pull the polygonal face
	// description directly (no coplanar-triangle recovery needed). For a cube vertex cloud the builder emits 6
	// quad faces, and CheckConvex should accept the result via the FaceLoops path.
	TArray<Chaos::FConvex::FVec3Type> InputVerts;
	InputVerts.Reserve(8);
	const double H = 5.0;
	InputVerts.Add(Chaos::FConvex::FVec3Type(-H, -H, -H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(+H, -H, -H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(+H, +H, -H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(-H, +H, -H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(-H, -H, +H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(+H, -H, +H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(+H, +H, +H));
	InputVerts.Add(Chaos::FConvex::FVec3Type(-H, +H, +H));

	TArray<Chaos::FConvex::FPlaneType> Planes;
	TArray<TArray<int32>> FaceIndices;
	TArray<Chaos::FConvex::FVec3Type> HullVerts;
	Chaos::FConvex::FAABB3Type LocalBounds;
	Chaos::FConvexBuilder::Build(InputVerts, Planes, FaceIndices, HullVerts, LocalBounds);

	Chaos::FConvexPtr ChaosPtr(new Chaos::FConvex(MoveTemp(Planes), MoveTemp(FaceIndices), MoveTemp(HullVerts)));

	// Pre-populate VertexData and IndexData so SetConvexMeshObject with UpdateConvexDataOnlyIfMissing leaves them alone —
	// the IndexData path isn't what we're testing here, we just need to satisfy FromChaosConvexHull's entry guard.
	FKConvexElem ConvexHull;
	for (const Chaos::FConvex::FVec3Type& V : InputVerts)
	{
		ConvexHull.VertexData.Add(FVector(V[0], V[1], V[2]));
	}
	ConvexHull.IndexData = { 0, 1, 2 };
	ConvexHull.SetConvexMeshObject(MoveTemp(ChaosPtr), FKConvexElem::EConvexDataUpdateMethod::UpdateConvexDataOnlyIfMissing);

	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromChaosConvexHull(ConvexHull, OutMesh);

	CHECK_MESSAGE(TEXT("FromChaosConvexHull should succeed with a live Chaos convex attached"), bResult);
	CHECK_EQUALS("Cube hull should yield 6 polygonal FaceLoops", OutMesh.FaceLoops.Num(), 6);
	for (int32 i = 0; i < OutMesh.FaceLoops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Each face from a cube hull should be a quad"), OutMesh.FaceLoops[i].IsQuad());
	}
	CHECK_MESSAGE(TEXT("Mesh built via Chaos face data should report convex"), OutMesh.IsConvex());
}

N_TEST_CRITICAL(FNRawMeshFactoryTests_FromChaosConvexHull_ValidData_PopulatesMesh,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosConvexHull::ValidData_PopulatesMesh",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKConvexElem ConvexHull;
	ConvexHull.VertexData = {
		FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0), FVector(0, 0, 10),
	};
	ConvexHull.IndexData = { 0, 1, 2,  0, 1, 3,  1, 2, 3,  2, 0, 3 };

	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromChaosConvexHull(ConvexHull, OutMesh);

	CHECK_MESSAGE(TEXT("FromChaosConvexHull should report success on valid input"), bResult);
	CHECK_EQUALS("Mesh should retain the four hull vertices", OutMesh.Vertices.Num(), 4);
	CHECK_EQUALS("Mesh should produce 4 triangles from 12 indices", OutMesh.Loops.Num(), 4);
	for (int32 i = 0; i < OutMesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop should be a triangle"), OutMesh.Loops[i].IsTriangle());
	}
	CHECK_MESSAGE(TEXT("Bounds should be valid after CalculateCenterAndBounds"), OutMesh.HasBounds());
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphere_VertexAndLoopCounts,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphere::VertexAndLoopCounts",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKSphereElem Sphere;
	Sphere.Radius = 10.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	const bool bResult = FNRawMeshFactory::FromChaosSphere(Sphere, FTransform::Identity, OutMesh, OutTransform);

	using namespace NEXUS::UnitTests::NCore::FNRawMeshFactoryHarness;
	const int32 ExpectedVerts = 2 + (SphereRings - 1) * SphereSegments;
	const int32 ExpectedLoops = SphereSegments * 2 + (SphereRings - 2) * SphereSegments * 2;

	CHECK_MESSAGE(TEXT("FromChaosSphere should report success"), bResult);
	CHECK_EQUALS("Sphere vertex count should equal 2 + (Rings-1)*Segments", OutMesh.Vertices.Num(), ExpectedVerts);
	CHECK_EQUALS("Sphere loop count should equal Segments*2 + (Rings-2)*Segments*2", OutMesh.Loops.Num(), ExpectedLoops);
	for (int32 i = 0; i < OutMesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop should be a triangle"), OutMesh.Loops[i].IsTriangle());
	}
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphere_VerticesAtRadius,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphere::VerticesAtRadius",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKSphereElem Sphere;
	Sphere.Radius = 25.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosSphere(Sphere, FTransform::Identity, OutMesh, OutTransform);

	bool bAllOnSurface = true;
	for (const FVector& V : OutMesh.Vertices)
	{
		if (!FMath::IsNearlyEqual(V.Size(), Sphere.Radius, 0.001))
		{
			bAllOnSurface = false;
			break;
		}
	}
	CHECK_MESSAGE(TEXT("Every emitted vertex should sit at the sphere's radius from the origin"), bAllOnSurface);
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphere_OutwardWindingReportsConvex,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphere::OutwardWindingReportsConvex",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A tessellated sphere is convex by construction. With outward-wound triangles every face plane has all other
	// vertices on the centroid side, so CheckConvex must report convex. Pre-fix the bands were wound inward and
	// this assertion would have failed regardless of FaceLoops.
	FKSphereElem Sphere;
	Sphere.Radius = 25.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosSphere(Sphere, FTransform::Identity, OutMesh, OutTransform);

	CHECK_MESSAGE(TEXT("Sphere with outward-wound bands should report convex via CheckConvex"), OutMesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphere_TransformCarriesCenter,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphere::TransformCarriesCenter",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKSphereElem Sphere;
	Sphere.Radius = 10.0;
	Sphere.Center = FVector(5, 6, 7);

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosSphere(Sphere, FTransform::Identity, OutMesh, OutTransform);

	CHECK_MESSAGE(TEXT("OutTransform translation should equal Sphere.Center when CompToWorld is identity"),
		OutTransform.GetTranslation().Equals(FVector(5, 6, 7), 0.001));
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphyl_VertexAndLoopCounts,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphyl::VertexAndLoopCounts",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKSphylElem Sphyl;
	Sphyl.Radius = 5.0;
	Sphyl.Length = 10.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	const bool bResult = FNRawMeshFactory::FromChaosSphyl(Sphyl, FTransform::Identity, OutMesh, OutTransform);

	using namespace NEXUS::UnitTests::NCore::FNRawMeshFactoryHarness;
	constexpr int32 TotalRings = 2 * SphylHemiRings;
	const int32 ExpectedVerts = 2 + TotalRings * SphereSegments;
	const int32 ExpectedLoops = SphereSegments * 2 + (TotalRings - 1) * SphereSegments * 2;

	CHECK_MESSAGE(TEXT("FromChaosSphyl should report success"), bResult);
	CHECK_EQUALS("Sphyl vertex count should equal 2 + (2*HemiRings)*Segments", OutMesh.Vertices.Num(), ExpectedVerts);
	CHECK_EQUALS("Sphyl loop count should equal Segments*2 + (TotalRings-1)*Segments*2", OutMesh.Loops.Num(), ExpectedLoops);
	for (int32 i = 0; i < OutMesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop should be a triangle"), OutMesh.Loops[i].IsTriangle());
	}
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphyl_FaceLoops_CylinderBandIsQuads,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphyl::FaceLoops_CylinderBandIsQuads",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The cylinder middle band is the only genuinely-planar-quad section of a sphyl (top/bottom edges are parallel chords
	// at fixed Z). FaceLoops should emit those as quads while keeping the saddle-shaped hemisphere bands and the pole fans
	// as triangles.
	FKSphylElem Sphyl;
	Sphyl.Radius = 5.0;
	Sphyl.Length = 10.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosSphyl(Sphyl, FTransform::Identity, OutMesh, OutTransform);

	using namespace NEXUS::UnitTests::NCore::FNRawMeshFactoryHarness;
	constexpr int32 TotalRings = 2 * SphylHemiRings;
	// Triangles: 2 cap fans (Segments each) + 2 hemisphere band groups, each (HemiRings - 1) bands of 2*Segments tris.
	constexpr int32 ExpectedTriFaces = SphereSegments * 2 + (TotalRings - 2) * SphereSegments * 2;
	constexpr int32 ExpectedQuadFaces = SphereSegments;

	int32 QuadCount = 0;
	int32 TriCount = 0;
	for (const FNRawMeshLoop& Face : OutMesh.FaceLoops)
	{
		if (Face.IsQuad()) QuadCount++;
		else if (Face.IsTriangle()) TriCount++;
	}

	CHECK_EQUALS("Sphyl FaceLoops should contain Segments quads for the cylinder middle band", QuadCount, ExpectedQuadFaces);
	CHECK_EQUALS("Sphyl FaceLoops should contain triangles for the pole fans and hemisphere bands", TriCount, ExpectedTriFaces);
	CHECK_MESSAGE(TEXT("Sphyl with outward-wound bands and FaceLoops should report convex"), OutMesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphyl_BoundsExtent,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphyl::BoundsExtent",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKSphylElem Sphyl;
	Sphyl.Radius = 5.0;
	Sphyl.Length = 10.0;

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosSphyl(Sphyl, FTransform::Identity, OutMesh, OutTransform);

	const double ExpectedHalfHeight = Sphyl.Length * 0.5 + Sphyl.Radius;

	CHECK_MESSAGE(TEXT("Sphyl Z extent should reach ±(Length/2 + Radius)"),
		FMath::IsNearlyEqual(OutMesh.Bounds.Max.Z, ExpectedHalfHeight, 0.001) &&
		FMath::IsNearlyEqual(OutMesh.Bounds.Min.Z, -ExpectedHalfHeight, 0.001));
	CHECK_MESSAGE(TEXT("Sphyl XY extent should reach ±Radius"),
		FMath::IsNearlyEqual(OutMesh.Bounds.Max.X, Sphyl.Radius, 0.001) &&
		FMath::IsNearlyEqual(OutMesh.Bounds.Min.X, -Sphyl.Radius, 0.001));
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosSphyl_TransformComposesCenterAndRotation,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosSphyl::TransformComposesCenterAndRotation",
	N_TEST_CONTEXT_ANYWHERE)
{
	FKSphylElem Sphyl;
	Sphyl.Radius = 5.0;
	Sphyl.Length = 10.0;
	Sphyl.Center = FVector(1, 2, 3);
	Sphyl.Rotation = FRotator(0.f, 0.f, 90.f);

	FNRawMesh OutMesh;
	FTransform OutTransform;
	FNRawMeshFactory::FromChaosSphyl(Sphyl, FTransform::Identity, OutMesh, OutTransform);

	CHECK_MESSAGE(TEXT("OutTransform translation should equal Sphyl.Center when CompToWorld is identity"),
		OutTransform.GetTranslation().Equals(FVector(1, 2, 3), 0.001));
	CHECK_MESSAGE(TEXT("OutTransform rotation should equal Sphyl.Rotation when CompToWorld is identity"),
		OutTransform.GetRotation().Equals(FQuat(FRotator(0.f, 0.f, 90.f)), 0.001));
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromStaticMesh_Null_False,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromStaticMesh::Null_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromStaticMesh(nullptr, OutMesh);

	CHECK_FALSE_MESSAGE(TEXT("Null UStaticMesh should produce false"), bResult);
	CHECK_EQUALS("Null UStaticMesh should leave OutMesh untouched", OutMesh.Vertices.Num(), 0);
}

N_TEST_CRITICAL(FNRawMeshFactoryTests_FromStaticMesh_EngineCube_ProducesGeometry,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromStaticMesh::EngineCube_ProducesGeometry",
	N_TEST_CONTEXT_EDITOR)
{
	// /Engine/BasicShapes/Cube is a built-in 8-vertex / 12-triangle static mesh — the render-data path
	// (route 1) should extract its LOD 0 positions and indices into a populated FNRawMesh. This is the
	// first end-to-end coverage of FromStaticMesh against a real UStaticMesh (the existing test only
	// covered the null-input guard).
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (Cube == nullptr)
	{
		ADD_ERROR("Failed to load /Engine/BasicShapes/Cube");
		return;
	}

	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromStaticMesh(Cube, OutMesh);

	CHECK_MESSAGE(TEXT("FromStaticMesh on a valid engine cube must report success"), bResult);
	CHECK_MESSAGE(TEXT("Engine cube must produce at least 8 vertices (one per corner; tessellation may add more)"),
		OutMesh.Vertices.Num() >= 8);
	CHECK_MESSAGE(TEXT("Engine cube must produce at least 12 triangle loops"), OutMesh.Loops.Num() >= 12);
	for (int32 i = 0; i < OutMesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop must be a triangle (Loops is post-CalculateFaceLoops triangulation)"),
			OutMesh.Loops[i].IsTriangle());
	}
	CHECK_MESSAGE(TEXT("Bounds must be populated after extraction"), OutMesh.HasBounds());
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosTriMeshes_NullPtr_False,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosTriMeshes::NullPtr_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Default-constructed Chaos::FTriangleMeshImplicitObjectPtr is null — the IsValid() guard at
	// NRawMeshFactory.cpp:341 must short-circuit and return false without touching OutMesh.
	const Chaos::FTriangleMeshImplicitObjectPtr NullPtr;
	FNRawMesh OutMesh;
	const bool bResult = FNRawMeshFactory::FromChaosTriMeshes(NullPtr, OutMesh);

	CHECK_FALSE_MESSAGE(TEXT("Null Chaos TriMesh ptr must produce false"), bResult);
	CHECK_EQUALS("Null Chaos TriMesh ptr must leave OutMesh untouched", OutMesh.Vertices.Num(), 0);
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromChaosBodySetup_EmptyTriMeshes_NoOutputAppended,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromChaosBodySetup::EmptyTriMeshes_NoOutputAppended",
	N_TEST_CONTEXT_EDITOR)
{
	// A freshly constructed UBodySetup has an empty TriMeshGeometries array — the loop iterates zero
	// times and nothing is appended. Covers the "no cooked tri meshes" path without requiring a real
	// physics-built body.
	UBodySetup* Body = NewObject<UBodySetup>();
	if (Body == nullptr)
	{
		ADD_ERROR("Failed to construct UBodySetup via NewObject");
		return;
	}

	TArray<FNRawMesh> OutMeshes;
	TArray<FTransform> OutTransforms;
	const bool bResult = FNRawMeshFactory::FromChaosBodySetup(Body, FTransform::Identity, OutMeshes, OutTransforms);

	CHECK_MESSAGE(TEXT("Empty TriMeshGeometries body should still return true (function returns true unconditionally after loop)"), bResult);
	CHECK_EQUALS("No meshes should be appended", OutMeshes.Num(), 0);
	CHECK_EQUALS("No transforms should be appended", OutTransforms.Num(), 0);
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromActorsInBounds_EmptyActors_NoOutput,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromActorsInBounds::EmptyActors_NoOutput",
	N_TEST_CONTEXT_EDITOR)
{
	// Empty actor list — function should run the WaitForStaticMeshCompilation pre-step and then the
	// outer loop iterates zero times, leaving the output arrays untouched.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		const TArray<AActor*> EmptyActors;
		const TArray<FBoxSphereBounds> EmptyBounds;
		TArray<FNRawMesh> OutMeshes;
		TArray<FTransform> OutTransforms;

		FNRawMeshFactory::FromActorsInBounds(EmptyActors, EmptyBounds, OutMeshes, OutTransforms);

		CHECK_EQUALS("Empty actor list must produce no meshes", OutMeshes.Num(), 0);
		CHECK_EQUALS("Empty actor list must produce no transforms", OutTransforms.Num(), 0);
	});
}

N_TEST_HIGH(FNRawMeshFactoryTests_FromActorsInBounds_BareActor_SkippedGracefully,
	"NEXUS::UnitTests::NCore::FNRawMeshFactory::FromActorsInBounds::BareActor_SkippedGracefully",
	N_TEST_CONTEXT_EDITOR)
{
	// A bare AActor (no primitive components → no UBoxComponent / UStaticMeshComponent) must be walked
	// without emitting any meshes and without crashing. Locks the inner TInlineComponentArray loop's
	// "no primitives" early-skip path. (A richer end-to-end test that emits meshes from a static-mesh
	// actor would need a deterministic UStaticMesh fixture with a known AggGeom — out of scope here.)
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		if (Actor == nullptr)
		{
			ADD_ERROR("Failed to spawn AActor");
			return;
		}

		const TArray<AActor*> Actors = { Actor };
		const TArray<FBoxSphereBounds> NoBounds;
		TArray<FNRawMesh> OutMeshes;
		TArray<FTransform> OutTransforms;

		FNRawMeshFactory::FromActorsInBounds(Actors, NoBounds, OutMeshes, OutTransforms);

		CHECK_EQUALS("A bare AActor must emit no meshes", OutMeshes.Num(), 0);
		CHECK_EQUALS("A bare AActor must emit no transforms", OutTransforms.Num(), 0);
	});
}

#endif //WITH_TESTS
