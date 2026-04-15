// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NTriangleUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNTriangleUtilsTests_IsPointInTriangle_Inside, "NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle_Inside", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector A(0.f, 0.f, 0.f);
	const FVector B(10.f, 0.f, 0.f);
	const FVector C(0.f, 10.f, 0.f);
	const FVector Point(2.f, 2.f, 0.f);
	CHECK_MESSAGE(TEXT("Point inside the triangle should return true"), FNTriangleUtils::IsPointInTriangle(Point, A, B, C));
}

N_TEST_CRITICAL(FNTriangleUtilsTests_IsPointInTriangle_Outside, "NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle_Outside", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector A(0.f, 0.f, 0.f);
	const FVector B(10.f, 0.f, 0.f);
	const FVector C(0.f, 10.f, 0.f);
	const FVector Point(10.f, 10.f, 0.f);
	CHECK_FALSE_MESSAGE(TEXT("Point outside the triangle should return false"), FNTriangleUtils::IsPointInTriangle(Point, A, B, C));
}

N_TEST_HIGH(FNTriangleUtilsTests_IsPointInTriangle_OnVertex, "NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle_OnVertex", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector A(0.f, 0.f, 0.f);
	const FVector B(10.f, 0.f, 0.f);
	const FVector C(0.f, 10.f, 0.f);
	CHECK_MESSAGE(TEXT("Point on vertex A should be inside"), FNTriangleUtils::IsPointInTriangle(A, A, B, C));
}

N_TEST_HIGH(FNTriangleUtilsTests_IsPointInTriangle_OnEdge, "NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle_OnEdge", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector A(0.f, 0.f, 0.f);
	const FVector B(10.f, 0.f, 0.f);
	const FVector C(0.f, 10.f, 0.f);
	const FVector EdgeMid(5.f, 0.f, 0.f);
	CHECK_MESSAGE(TEXT("Point on edge midpoint should be inside"), FNTriangleUtils::IsPointInTriangle(EdgeMid, A, B, C));
}

N_TEST_HIGH(FNTriangleUtilsTests_IsPointInTriangle_Centroid, "NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle_Centroid", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector A(0.f, 0.f, 0.f);
	const FVector B(6.f, 0.f, 0.f);
	const FVector C(3.f, 6.f, 0.f);
	const FVector Centroid = (A + B + C) / 3.f;
	CHECK_MESSAGE(TEXT("Centroid should always be inside the triangle"), FNTriangleUtils::IsPointInTriangle(Centroid, A, B, C));
}

N_TEST_CRITICAL(FNTriangleUtilsTests_TrianglesIntersect_Overlapping, "NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect_Overlapping", N_TEST_CONTEXT_ANYWHERE)
{
	// Two overlapping triangles in the XY plane
	const FVector V0(0.f, 0.f, 0.f), V1(10.f, 0.f, 0.f), V2(5.f, 10.f, 0.f);
	const FVector U0(3.f, 2.f, 0.f), U1(13.f, 2.f, 0.f), U2(8.f, 12.f, 0.f);
	CHECK_MESSAGE(TEXT("Overlapping triangles should intersect"), FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2));
}

N_TEST_CRITICAL(FNTriangleUtilsTests_TrianglesIntersect_Separated, "NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect_Separated", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector V0(0.f, 0.f, 0.f), V1(1.f, 0.f, 0.f), V2(0.f, 1.f, 0.f);
	const FVector U0(100.f, 100.f, 0.f), U1(101.f, 100.f, 0.f), U2(100.f, 101.f, 0.f);
	CHECK_FALSE_MESSAGE(TEXT("Widely separated triangles should not intersect"), FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_CrossingPlanes, "NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect_CrossingPlanes", N_TEST_CONTEXT_ANYWHERE)
{
	// Triangles that cross each other in 3D
	const FVector V0(-5.f, 0.f, 0.f), V1(5.f, 0.f, 0.f), V2(0.f, 5.f, 0.f);
	const FVector U0(0.f, 2.f, -5.f), U1(0.f, 2.f, 5.f), U2(0.f, -3.f, 0.f);
	CHECK_MESSAGE(TEXT("Triangles crossing in 3D should intersect"), FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_ParallelNoOverlap, "NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect_ParallelNoOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	// Two parallel triangles offset on Z axis
	const FVector V0(0.f, 0.f, 0.f), V1(10.f, 0.f, 0.f), V2(5.f, 10.f, 0.f);
	const FVector U0(0.f, 0.f, 5.f), U1(10.f, 0.f, 5.f), U2(5.f, 10.f, 5.f);
	CHECK_FALSE_MESSAGE(TEXT("Parallel non-touching triangles should not intersect"), FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_SharedEdge, "NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect_SharedEdge", N_TEST_CONTEXT_ANYWHERE)
{
	// Two coplanar triangles sharing an edge
	const FVector V0(0.f, 0.f, 0.f), V1(10.f, 0.f, 0.f), V2(5.f, 10.f, 0.f);
	const FVector U0(0.f, 0.f, 0.f), U1(10.f, 0.f, 0.f), U2(5.f, -10.f, 0.f);
	CHECK_MESSAGE(TEXT("Coplanar triangles sharing an edge should intersect"), FNTriangleUtils::TrianglesIntersect(V0, V1, V2, U0, U1, U2));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_Identical, "NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect_Identical", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector V0(0.f, 0.f, 0.f), V1(10.f, 0.f, 0.f), V2(5.f, 10.f, 0.f);
	CHECK_MESSAGE(TEXT("Identical triangles should intersect"), FNTriangleUtils::TrianglesIntersect(V0, V1, V2, V0, V1, V2));
}

#endif //WITH_TESTS
