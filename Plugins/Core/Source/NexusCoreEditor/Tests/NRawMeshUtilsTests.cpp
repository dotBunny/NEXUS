// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness
{
	/** Builds a triangle-tessellated cube with outward-facing winding centred on the origin. */
	static FNRawMesh MakeCube(const double HalfExtent)
	{
		FNRawMesh Mesh;
		const double H = HalfExtent;
		Mesh.Vertices = {
			{ -H, -H, -H }, { +H, -H, -H }, { +H, +H, -H }, { -H, +H, -H },
			{ -H, -H, +H }, { +H, -H, +H }, { +H, +H, +H }, { -H, +H, +H },
		};
		static const int32 Tris[12][3] = {
			{ 0, 2, 1 }, { 0, 3, 2 },
			{ 4, 5, 6 }, { 4, 6, 7 },
			{ 0, 1, 5 }, { 0, 5, 4 },
			{ 1, 2, 6 }, { 1, 6, 5 },
			{ 2, 3, 7 }, { 2, 7, 6 },
			{ 3, 0, 4 }, { 3, 4, 7 },
		};
		for (int32 i = 0; i < 12; ++i)
		{
			Mesh.Loops.Add(FNRawMeshLoop(Tris[i][0], Tris[i][1], Tris[i][2]));
		}
		Mesh.CalculateCenterAndBounds();
		Mesh.Validate();
		return Mesh;
	}
}

N_TEST_HIGH(FNRawMeshUtilsTests_CombineMesh_EmptyOther_NoOp,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::EmptyOther_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Base = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	const int32 OriginalVertexCount = Base.Vertices.Num();
	const int32 OriginalLoopCount = Base.Loops.Num();

	const FNRawMesh Empty;
	FNRawMeshUtils::CombineMesh(FTransform::Identity, Base, FTransform::Identity, Empty);

	CHECK_EQUALS("Empty Other should leave the Base vertex count unchanged", Base.Vertices.Num(), OriginalVertexCount);
	CHECK_EQUALS("Empty Other should leave the Base loop count unchanged", Base.Loops.Num(), OriginalLoopCount);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_CombineMesh_Identity_AppendsAndShiftsIndices,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::Identity_AppendsAndShiftsIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Base = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	const int32 BaseVertexCount = Base.Vertices.Num();
	const int32 BaseLoopCount = Base.Loops.Num();

	FNRawMesh Other;
	Other.Vertices = { FVector(100, 0, 0), FVector(110, 0, 0), FVector(100, 10, 0) };
	Other.Loops.Add(FNRawMeshLoop(0, 1, 2));

	FNRawMeshUtils::CombineMesh(FTransform::Identity, Base, FTransform::Identity, Other);

	CHECK_EQUALS("Combined mesh should contain Base + Other vertices", Base.Vertices.Num(), BaseVertexCount + 3);
	CHECK_EQUALS("Combined mesh should contain Base + Other loops", Base.Loops.Num(), BaseLoopCount + 1);
	CHECK_MESSAGE(TEXT("Identity transforms should preserve Other's first vertex position"),
		Base.Vertices[BaseVertexCount].Equals(FVector(100, 0, 0), 0.001));

	const FNRawMeshLoop& AppendedLoop = Base.Loops.Last();
	CHECK_EQUALS("Appended loop's first index should be shifted past Base's existing vertices", AppendedLoop.Indices[0], BaseVertexCount + 0);
	CHECK_EQUALS("Appended loop's second index should be shifted past Base's existing vertices", AppendedLoop.Indices[1], BaseVertexCount + 1);
	CHECK_EQUALS("Appended loop's third index should be shifted past Base's existing vertices", AppendedLoop.Indices[2], BaseVertexCount + 2);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_CombineMesh_FaceLoops_BothSidesHaveThem_AppendedWithShift,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::FaceLoops_BothSidesHaveThem_AppendedWithShift",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When both inputs carry a polygonal FaceLoops description, the merged mesh should keep both — with
	// Other's indices shifted past Base's vertex count, exactly the way Loops are shifted. Without this,
	// CheckConvex on the merged mesh falls back to walking the fan-triangulated Loops and re-introduces
	// the per-triangle coplanar drift the FaceLoops path was added to avoid.
	FNRawMesh Base;
	Base.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Base.FaceLoops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Base.Loops = Base.FaceLoops;

	FNRawMesh Other;
	Other.Vertices = { FVector(100, 0, 0), FVector(110, 0, 0), FVector(110, 10, 0), FVector(100, 10, 0) };
	Other.FaceLoops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Other.Loops = Other.FaceLoops;

	const int32 BaseVertexCount = Base.Vertices.Num();
	const int32 BaseFaceLoopCount = Base.FaceLoops.Num();

	FNRawMeshUtils::CombineMesh(FTransform::Identity, Base, FTransform::Identity, Other);

	CHECK_EQUALS("Combined FaceLoops count should be Base + Other", Base.FaceLoops.Num(), BaseFaceLoopCount + 1);

	const FNRawMeshLoop& AppendedFace = Base.FaceLoops.Last();
	CHECK_EQUALS("Appended FaceLoop should preserve the 4-vertex polygon", AppendedFace.Indices.Num(), 4);
	CHECK_EQUALS("Appended FaceLoop index 0 should be shifted by VertexOffset", AppendedFace.Indices[0], BaseVertexCount + 0);
	CHECK_EQUALS("Appended FaceLoop index 1 should be shifted by VertexOffset", AppendedFace.Indices[1], BaseVertexCount + 1);
	CHECK_EQUALS("Appended FaceLoop index 2 should be shifted by VertexOffset", AppendedFace.Indices[2], BaseVertexCount + 2);
	CHECK_EQUALS("Appended FaceLoop index 3 should be shifted by VertexOffset", AppendedFace.Indices[3], BaseVertexCount + 3);
}

N_TEST_HIGH(FNRawMeshUtilsTests_CombineMesh_FaceLoops_OnlyBaseHasThem_Cleared,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::FaceLoops_OnlyBaseHasThem_Cleared",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Partial FaceLoops coverage would let CheckConvex skip the undocumented side's face planes and falsely
	// accept non-convex merges. The combined mesh must drop FaceLoops entirely when either side lacks it.
	FNRawMesh Base;
	Base.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Base.FaceLoops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Base.Loops = Base.FaceLoops;

	FNRawMesh Other;
	Other.Vertices = { FVector(100, 0, 0), FVector(110, 0, 0), FVector(105, 10, 0) };
	Other.Loops.Add(FNRawMeshLoop(0, 1, 2));
	// Deliberately leave Other.FaceLoops empty.

	FNRawMeshUtils::CombineMesh(FTransform::Identity, Base, FTransform::Identity, Other);

	CHECK_EQUALS("Partial-coverage merge must clear FaceLoops rather than leave a half-described mesh", Base.FaceLoops.Num(), 0);
}

N_TEST_HIGH(FNRawMeshUtilsTests_CombineMesh_FaceLoops_OnlyOtherHasThem_Cleared,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::FaceLoops_OnlyOtherHasThem_Cleared",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mirror of the previous test: Base lacks FaceLoops, Other has it. Still a partial-coverage case
	// and the combined mesh must clear FaceLoops.
	FNRawMesh Base;
	Base.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(5, 10, 0) };
	Base.Loops.Add(FNRawMeshLoop(0, 1, 2));
	// Deliberately leave Base.FaceLoops empty.

	FNRawMesh Other;
	Other.Vertices = { FVector(100, 0, 0), FVector(110, 0, 0), FVector(110, 10, 0), FVector(100, 10, 0) };
	Other.FaceLoops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Other.Loops = Other.FaceLoops;

	FNRawMeshUtils::CombineMesh(FTransform::Identity, Base, FTransform::Identity, Other);

	CHECK_EQUALS("Partial-coverage merge must clear FaceLoops regardless of which side carried the description", Base.FaceLoops.Num(), 0);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_CombineMesh_TransformsCompose,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::TransformsCompose",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Other lives at world (50, 0, 0); Base lives at world (10, 0, 0). Other's vertex at (1, 0, 0) local should land in
	// Base-local space at (50 + 1) - 10 = (41, 0, 0).
	FNRawMesh Base;
	Base.Vertices.Add(FVector(0, 0, 0));

	FNRawMesh Other;
	Other.Vertices = { FVector(1, 0, 0) };
	Other.Loops.Add(FNRawMeshLoop(0, 0, 0));

	const FTransform BaseTransform(FQuat::Identity, FVector(10, 0, 0));
	const FTransform OtherTransform(FQuat::Identity, FVector(50, 0, 0));

	FNRawMeshUtils::CombineMesh(BaseTransform, Base, OtherTransform, Other);

	CHECK_MESSAGE(TEXT("Other's vertex should be re-expressed in Base-local space using OtherTransform * BaseTransform.Inverse()"),
		Base.Vertices[1].Equals(FVector(41, 0, 0), 0.001));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_Center_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::Center_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	CHECK_MESSAGE(TEXT("Origin should be reported inside a cube centred on the origin"),
		FNRawMeshUtils::IsRelativePointInside(Cube, FVector::ZeroVector));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_Outside_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::Outside_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	CHECK_FALSE_MESSAGE(TEXT("Point well outside the cube must be reported as outside"),
		FNRawMeshUtils::IsRelativePointInside(Cube, FVector(100, 0, 0)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_OnFace_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::OnFace_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Boundary point — DotProduct(Normal, Point - V0) is zero, so CenterSide * PointSide == 0 (not < 0) and the test passes.
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	CHECK_MESSAGE(TEXT("Point exactly on a face should be reported as inside (boundary)"),
		FNRawMeshUtils::IsRelativePointInside(Cube, FVector(5, 0, 0)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonConvex_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonConvex_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("not convex or has non-triangles"), ELogVerbosity::Warning);

	// Concave pentagon — IsConvex() returns false so the guard rejects the input.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(0,  0,  0),
		FVector(10, 5,  0),
		FVector(0,  10, 0),
		FVector(2,  7,  0),
		FVector(2,  3,  0)
	};
	Mesh.Loops.Add(FNRawMeshLoop(TArray<int32>{0, 1, 2, 3, 4}));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Non-convex mesh must be rejected by IsRelativePointInside"),
		FNRawMeshUtils::IsRelativePointInside(Mesh, FVector(1, 1, 0)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonTri_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonTri_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("not convex or has non-triangles"), ELogVerbosity::Warning);

	// Convex but quad-based — guard must still reject for HasNonTris().
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Quad-based mesh must be rejected by IsRelativePointInside"),
		FNRawMeshUtils::IsRelativePointInside(Mesh, FVector(5, 5, 0)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_AnyRelativePointsInside_AllOutside_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::AnyRelativePointsInside::AllOutside_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	const TArray<FVector> Points = { FVector(100, 0, 0), FVector(-100, 0, 0), FVector(0, 0, 100) };
	CHECK_FALSE_MESSAGE(TEXT("All points outside should produce false"),
		FNRawMeshUtils::AnyRelativePointsInside(Cube, Points));
}

N_TEST_HIGH(FNRawMeshUtilsTests_AnyRelativePointsInside_OneInside_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::AnyRelativePointsInside::OneInside_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mid-list inside point — confirms the function actually iterates past the first-element short-circuit candidate.
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	const TArray<FVector> Points = { FVector(100, 0, 0), FVector(0, 0, 0), FVector(-100, 0, 0) };
	CHECK_MESSAGE(TEXT("At least one inside point should produce true"),
		FNRawMeshUtils::AnyRelativePointsInside(Cube, Points));
}

N_TEST_HIGH(FNRawMeshUtilsTests_AnyRelativePointsInside_EmptyArray_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::AnyRelativePointsInside::EmptyArray_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	const TArray<FVector> Empty;
	CHECK_FALSE_MESSAGE(TEXT("Empty point list should produce false"),
		FNRawMeshUtils::AnyRelativePointsInside(Cube, Empty));
}

N_TEST_HIGH(FNRawMeshUtilsTests_AnyRelativePointsInside_NonConvex_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::AnyRelativePointsInside::NonConvex_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("not convex or has non-triangles"), ELogVerbosity::Warning);

	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(0,  0,  0),
		FVector(10, 5,  0),
		FVector(0,  10, 0),
		FVector(2,  7,  0),
		FVector(2,  3,  0)
	};
	Mesh.Loops.Add(FNRawMeshLoop(TArray<int32>{0, 1, 2, 3, 4}));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	const TArray<FVector> Points = { FVector(1, 1, 0) };
	CHECK_FALSE_MESSAGE(TEXT("Non-convex mesh must be rejected by AnyRelativePointsInside"),
		FNRawMeshUtils::AnyRelativePointsInside(Mesh, Points));
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_DoesIntersect_SeparatedAABBs_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::SeparatedAABBs_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Cube, FVector(0, 0, 0), FRotator::ZeroRotator,
		Cube, FVector(1000, 0, 0), FRotator::ZeroRotator);

	CHECK_FALSE_MESSAGE(TEXT("Far-apart AABBs should be rejected by the early-out before triangle tests run"), bResult);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_DoesIntersect_OverlappingCubes_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::OverlappingCubes_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	// Right cube shifted by 5 along X — its left face passes through the centre of the left cube.
	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Cube, FVector(0, 0, 0), FRotator::ZeroRotator,
		Cube, FVector(5, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Half-overlapping cubes must be reported as intersecting"), bResult);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_DoesIntersect_FullyContained_TrueViaContainmentFallback,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::FullyContained_TrueViaContainmentFallback",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Small cube fully inside a large cube — no triangles cross, so this path can only succeed via the
	// "sample one vertex" containment fallback at the end of DoesIntersectTriangles.
	const FNRawMesh Big = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(10.0);
	const FNRawMesh Small = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(1.0);

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Big,   FVector(0, 0, 0), FRotator::ZeroRotator,
		Small, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Inner cube fully contained by outer cube must be reported as intersecting via the containment fallback"), bResult);
}

N_TEST_HIGH(FNRawMeshUtilsTests_DoesIntersect_RotatedOverlap_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::RotatedOverlap_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Cube, FVector(0, 0, 0), FRotator::ZeroRotator,
		Cube, FVector(0, 0, 0), FRotator(0.f, 45.f, 0.f));

	CHECK_MESSAGE(TEXT("Co-located cubes with a yaw offset must still be reported as intersecting"), bResult);
}

N_TEST_HIGH(FNRawMeshUtilsTests_DoesIntersect_EmptyLoops_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::EmptyLoops_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("No loops were found"), ELogVerbosity::Warning);

	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	FNRawMesh Empty;

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Cube,  FVector(0, 0, 0), FRotator::ZeroRotator,
		Empty, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_FALSE_MESSAGE(TEXT("Loop-less mesh must be rejected with a warning"), bResult);
}

N_TEST_HIGH(FNRawMeshUtilsTests_DoesIntersect_NonTriMesh_FalseWithError,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::NonTriMesh_FalseWithError",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("non-triangle based geometry"), ELogVerbosity::Error);

	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	FNRawMesh Quad;
	Quad.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Quad.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Quad.CalculateCenterAndBounds();
	Quad.Validate();

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Cube, FVector(0, 0, 0), FRotator::ZeroRotator,
		Quad, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_FALSE_MESSAGE(TEXT("Quad-based input must be rejected with an error"), bResult);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_SeparatedAABBs_NegativeOne,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::SeparatedAABBs_NegativeOne",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Cube, FVector(0, 0, 0), FRotator::ZeroRotator,
		Cube, FVector(1000, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Far-apart AABBs must return the -1 'no overlap' sentinel"),
		FMath::IsNearlyEqual(Depth, -1.0f, 0.001f));
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_PartialOverlap_NearestFaceDistance,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::PartialOverlap_NearestFaceDistance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Small Right cube (HalfExtent 5) shifted along +X into a Large Left cube (HalfExtent 10).
	// Right's -X face vertices land at world x=7 (inside Left, which spans -10..+10). The deepest
	// face-plane distance from those vertices is to Left's +X face at x=+10 → depth = 3.
	const FNRawMesh Left = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(10.0);
	const FNRawMesh Right = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Left,  FVector(0,  0, 0), FRotator::ZeroRotator,
		Right, FVector(12, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Partial overlap depth should match the closest-face perpendicular distance (3.0)"),
		FMath::IsNearlyEqual(Depth, 3.0f, 0.001f));
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_FullyContained_VertexInsetDistance,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::FullyContained_VertexInsetDistance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Tiny Right cube (HalfExtent 1) fully inside Large Left cube (HalfExtent 10). Each Right vertex
	// sits 9 units away from Left's nearest face → depth = 9.
	const FNRawMesh Left  = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(10.0);
	const FNRawMesh Right = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(1.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Left,  FVector(0, 0, 0), FRotator::ZeroRotator,
		Right, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Fully-contained Right should report Right-vertex-to-Left-face distance (9.0)"),
		FMath::IsNearlyEqual(Depth, 9.0f, 0.001f));
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_SymmetricSwap_SameResult,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::SymmetricSwap_SameResult",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Swapping Left and Right must yield the same depth — the metric is symmetric by construction
	// (it picks the larger of the two vertex-in-other measurements).
	const FNRawMesh Big   = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(10.0);
	const FNRawMesh Small = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(1.0);

	const float DepthA = FNRawMeshUtils::GetIntersectDepth(
		Big,   FVector(0, 0, 0), FRotator::ZeroRotator,
		Small, FVector(0, 0, 0), FRotator::ZeroRotator);
	const float DepthB = FNRawMeshUtils::GetIntersectDepth(
		Small, FVector(0, 0, 0), FRotator::ZeroRotator,
		Big,   FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Depth result must not depend on which mesh is passed as Left vs Right"),
		FMath::IsNearlyEqual(DepthA, DepthB, 0.001f));
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_EmptyLoops_NegativeOneWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::EmptyLoops_NegativeOneWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("No loops were found"), ELogVerbosity::Warning);

	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	FNRawMesh Empty;

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Cube,  FVector(0, 0, 0), FRotator::ZeroRotator,
		Empty, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Loop-less mesh must yield the -1 sentinel with a warning"),
		FMath::IsNearlyEqual(Depth, -1.0f, 0.001f));
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_NonTriMesh_NegativeOneWithError,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::NonTriMesh_NegativeOneWithError",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("non-triangle based geometry"), ELogVerbosity::Error);

	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);

	FNRawMesh Quad;
	Quad.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Quad.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Quad.CalculateCenterAndBounds();
	Quad.Validate();

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Cube, FVector(0, 0, 0), FRotator::ZeroRotator,
		Quad, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Quad-based input must yield the -1 sentinel with an error"),
		FMath::IsNearlyEqual(Depth, -1.0f, 0.001f));
}

#endif //WITH_TESTS
