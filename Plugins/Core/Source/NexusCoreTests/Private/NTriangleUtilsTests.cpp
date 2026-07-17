// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NTriangleUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

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

N_TEST_HIGH(FNTriangleUtilsTests_IsPointInTriangle_Degenerate_FalseViaDenomGuard,
	"NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle::Degenerate_FalseViaDenomGuard",
	N_TEST_CONTEXT_ANYWHERE)
{
	// All three vertices collinear → the barycentric denominator collapses to 0 and the SMALL_NUMBER guard
	// at NTriangleUtils.h:35 must return false. Without that guard the divide would produce NaN/Inf and the
	// (u >= 0 && v >= 0 && u+v <= 1) check would have undefined behaviour.
	const FVector A(0, 0, 0);
	const FVector B(5, 0, 0);
	const FVector C(10, 0, 0);
	CHECK_FALSE_MESSAGE(TEXT("Collinear-vertex triangle must reject any test point via the denom guard"),
		FNTriangleUtils::IsPointInTriangle(FVector(3, 0, 0), A, B, C));
}

N_TEST_HIGH(FNTriangleUtilsTests_IsPointInTriangle_OffPlane_StillEvaluatesBarycentric,
	"NEXUS::UnitTests::NCore::FNTriangleUtils::IsPointInTriangle::OffPlane_StillEvaluatesBarycentric",
	N_TEST_CONTEXT_ANYWHERE)
{
	// IsPointInTriangle does NOT perform a plane-distance check — it computes barycentric coordinates
	// against the triangle's basis and accepts the point as long as (u, v) land inside the simplex. A point
	// directly above the centroid therefore reports inside; callers needing plane-distance rejection must
	// project first. This test locks that behaviour so callers don't get surprised.
	const FVector A(0, 0, 0);
	const FVector B(10, 0, 0);
	const FVector C(0, 10, 0);
	const FVector AbovePoint(2, 2, 1000);  // Same XY as a clearly-inside point, but way above the plane.
	CHECK_MESSAGE(TEXT("Off-plane point with in-triangle XY must still report true — caller is responsible for plane projection"),
		FNTriangleUtils::IsPointInTriangle(AbovePoint, A, B, C));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_Coplanar_OneInsideOther_True,
	"NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect::Coplanar_OneInsideOther_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Inner triangle entirely contained within outer triangle, coplanar — no edges cross, so the Möller
	// algorithm's coplanar branch must handle containment correctly (otherwise it would falsely report no
	// intersection because no edge-edge tests succeed).
	const FVector Outer0(0, 0, 0), Outer1(20, 0, 0), Outer2(10, 20, 0);
	const FVector Inner0(5, 2, 0), Inner1(15, 2, 0), Inner2(10, 8, 0);
	CHECK_MESSAGE(TEXT("Coplanar inner-triangle-inside-outer must report intersect"),
		FNTriangleUtils::TrianglesIntersect(Outer0, Outer1, Outer2, Inner0, Inner1, Inner2));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_CoplanarSeparated_False,
	"NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect::CoplanarSeparated_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two coplanar triangles 100 units apart — the coplanar branch's edge-edge tests must reject when
	// neither edge crosses and neither triangle contains the other's vertices. Distinct from the existing
	// Separated test (which has off-plane geometry caught by the plane-distance rejection).
	const FVector A0(0, 0, 0), A1(10, 0, 0), A2(5, 10, 0);
	const FVector B0(100, 0, 0), B1(110, 0, 0), B2(105, 10, 0);
	CHECK_FALSE_MESSAGE(TEXT("Coplanar but spatially separated triangles must not report intersect"),
		FNTriangleUtils::TrianglesIntersect(A0, A1, A2, B0, B1, B2));
}

N_TEST_HIGH(FNTriangleUtilsTests_TrianglesIntersect_SharedVertexOnly_True,
	"NEXUS::UnitTests::NCore::FNTriangleUtils::TrianglesIntersect::SharedVertexOnly_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two coplanar triangles sharing exactly one vertex with no other overlap — Möller's coplanar branch
	// treats a shared point as a degenerate interval overlap, so this must report true (matches the
	// SharedEdge behaviour). A bug that excluded boundary touches would surface here.
	const FVector Shared(0, 0, 0);
	const FVector A1(10, 0, 0), A2(5, 10, 0);
	const FVector B1(-10, 0, 0), B2(-5, -10, 0);
	CHECK_MESSAGE(TEXT("Coplanar triangles touching only at a single shared vertex must report intersect"),
		FNTriangleUtils::TrianglesIntersect(Shared, A1, A2, Shared, B1, B2));
}

#endif //WITH_TESTS
