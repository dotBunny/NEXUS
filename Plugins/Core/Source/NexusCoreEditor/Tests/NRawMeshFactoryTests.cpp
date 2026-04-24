// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMeshFactory.h"
#include "Macros/NTestMacros.h"
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

#endif //WITH_TESTS
