// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMeshUtils.h"
#include "Types/NRawMeshFactory.h"
#include "Developer/NTestUtils.h"
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

	/** Builds an axis-aligned box with arbitrary half-extents using the same hand-rolled topology as MakeCube. */
	static FNRawMesh MakeBox(const double Hx, const double Hy, const double Hz)
	{
		FNRawMesh Mesh;
		Mesh.Vertices = {
			{ -Hx, -Hy, -Hz }, { +Hx, -Hy, -Hz }, { +Hx, +Hy, -Hz }, { -Hx, +Hy, -Hz },
			{ -Hx, -Hy, +Hz }, { +Hx, -Hy, +Hz }, { +Hx, +Hy, +Hz }, { -Hx, +Hy, +Hz },
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

	/** Emits a box via FromChaosBox so depth tests exercise the factory production path (FaceLoops populated, bIsChaosGenerated=true). */
	static FNRawMesh MakeChaosBox(const double FullX, const double FullY, const double FullZ)
	{
		FKBoxElem Box;
		Box.X = FullX;
		Box.Y = FullY;
		Box.Z = FullZ;
		FNRawMesh OutMesh;
		FTransform OutTransform;
		FNRawMeshFactory::FromChaosBox(Box, FTransform::Identity, OutMesh, OutTransform);
		return OutMesh;
	}

	/** Emits a UV-tessellated sphere via FromChaosSphere. */
	static FNRawMesh MakeChaosSphere(const double Radius)
	{
		FKSphereElem Sphere;
		Sphere.Radius = Radius;
		FNRawMesh OutMesh;
		FTransform OutTransform;
		FNRawMeshFactory::FromChaosSphere(Sphere, FTransform::Identity, OutMesh, OutTransform);
		return OutMesh;
	}

	/** Emits a tessellated capsule (cylinder band + hemispherical caps) via FromChaosSphyl. */
	static FNRawMesh MakeChaosSphyl(const double Radius, const double Length)
	{
		FKSphylElem Sphyl;
		Sphyl.Radius = Radius;
		Sphyl.Length = Length;
		FNRawMesh OutMesh;
		FTransform OutTransform;
		FNRawMeshFactory::FromChaosSphyl(Sphyl, FTransform::Identity, OutMesh, OutTransform);
		return OutMesh;
	}

	/**
	 * Triangulated non-convex closed manifold: a "notched-square" pentagon (the unit square with a
	 * triangular bite taken out of the top edge) extruded along +Z. Used to exercise the non-convex
	 * fallback in IsRelativePointInside / DoesIntersect.
	 *
	 * Footprint (z=0 and z=5):
	 *   (0,10) ----------- (10,10)
	 *      \              /
	 *       \   notch    /
	 *        \          /
	 *         \ (5,5)  /
	 *         /        \             ← interior continues here
	 *        /          \
	 *   (0,0) ---------- (10,0)
	 *
	 * Walking the bottom polygon CCW from +Z: 0,1,2,3,4 where 3=(5,5) is the concave (inner) vertex.
	 * The polygon area is 75 (10×10 square minus the 25-unit notch triangle); points like (3,3) are
	 * inside the body, points like (5,8) sit in the cut-out and are outside.
	 */
	static FNRawMesh MakeNonConvexPrism()
	{
		FNRawMesh Mesh;
		Mesh.Vertices = {
			{ 0,  0, 0 }, { 10,  0, 0 }, { 10, 10, 0 }, { 5,  5, 0 }, { 0, 10, 0 },
			{ 0,  0, 5 }, { 10,  0, 5 }, { 10, 10, 5 }, { 5,  5, 5 }, { 0, 10, 5 },
		};

		// Bottom (outward -Z, CW from +Z view). Fan from the notch vertex 3.
		Mesh.Loops.Add(FNRawMeshLoop(3, 2, 1));
		Mesh.Loops.Add(FNRawMeshLoop(3, 1, 0));
		Mesh.Loops.Add(FNRawMeshLoop(3, 0, 4));

		// Top (outward +Z, CCW from +Z view). Fan from the notch vertex 8.
		Mesh.Loops.Add(FNRawMeshLoop(8, 9, 5));
		Mesh.Loops.Add(FNRawMeshLoop(8, 5, 6));
		Mesh.Loops.Add(FNRawMeshLoop(8, 6, 7));

		// Five rectangular sides, each two triangles, wound (A, B, B', A') for outward winding.
		Mesh.Loops.Add(FNRawMeshLoop(0, 1, 6)); Mesh.Loops.Add(FNRawMeshLoop(0, 6, 5)); // edge 0→1, outward -Y
		Mesh.Loops.Add(FNRawMeshLoop(1, 2, 7)); Mesh.Loops.Add(FNRawMeshLoop(1, 7, 6)); // edge 1→2, outward +X
		Mesh.Loops.Add(FNRawMeshLoop(2, 3, 8)); Mesh.Loops.Add(FNRawMeshLoop(2, 8, 7)); // edge 2→3, notch wall
		Mesh.Loops.Add(FNRawMeshLoop(3, 4, 9)); Mesh.Loops.Add(FNRawMeshLoop(3, 9, 8)); // edge 3→4, notch wall
		Mesh.Loops.Add(FNRawMeshLoop(4, 0, 5)); Mesh.Loops.Add(FNRawMeshLoop(4, 5, 9)); // edge 4→0, outward -X

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

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonTriPentagon_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonTriPentagon_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("has non-triangle loops"), ELogVerbosity::Warning);

	// A 5-vertex single-loop pentagon: rejected by the HasNonTris() guard. (Non-convex meshes
	// themselves are no longer rejected — they fall through to the parity ray-cast path — but a
	// non-triangle loop still is, because both code paths assume triangle topology.)
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

	CHECK_FALSE_MESSAGE(TEXT("Pentagon loop is non-triangle and must be rejected by IsRelativePointInside"),
		FNRawMeshUtils::IsRelativePointInside(Mesh, FVector(1, 1, 0)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonTri_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonTri_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("has non-triangle loops"), ELogVerbosity::Warning);

	// Convex quad-based mesh — the only remaining rejection condition is HasNonTris().
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

N_TEST_HIGH(FNRawMeshUtilsTests_AnyRelativePointsInside_NonTriPentagon_FalseWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::AnyRelativePointsInside::NonTriPentagon_FalseWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("has non-triangle loops"), ELogVerbosity::Warning);

	// Mirrors IsRelativePointInside::NonTriPentagon — AnyRelativePointsInside delegates and must
	// surface the same HasNonTris() rejection.
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
	CHECK_FALSE_MESSAGE(TEXT("Pentagon loop is non-triangle and must be rejected by AnyRelativePointsInside"),
		FNRawMeshUtils::AnyRelativePointsInside(Mesh, Points));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonConvex_BodyPoint_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonConvex_BodyPoint_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Notched-square prism is a triangulated, closed, non-convex manifold; IsRelativePointInside
	// drops into the parity-raycast fallback. Point (3, 3, 2.5) is inside the body of the L —
	// the +X probe ray crosses exactly one wall (the +X side of the square), giving odd parity.
	const FNRawMesh Mesh = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();

	CHECK_FALSE_MESSAGE(TEXT("Sanity: the notched prism must report IsConvex==false so the ray-cast fallback path runs"),
		Mesh.IsConvex());
	CHECK_MESSAGE(TEXT("Point inside the prism's solid body should ray-cast to odd parity (inside)"),
		FNRawMeshUtils::IsRelativePointInside(Mesh, FVector(3, 3, 2.5)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonConvex_NotchPoint_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonConvex_NotchPoint_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Point (5, 8, 2.5) sits inside the prism's AABB but in the cut-out region of the footprint —
	// the +X probe ray crosses two walls (the +X edge of the square, plus the notch-2→3 wall),
	// giving even parity. This is the case that motivates the non-convex fallback: a convex
	// half-space test would falsely call it "inside" because the AABB hides the indent.
	const FNRawMesh Mesh = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();

	CHECK_FALSE_MESSAGE(TEXT("Point in the cut-out region of the notched prism must report outside"),
		FNRawMeshUtils::IsRelativePointInside(Mesh, FVector(5, 8, 2.5)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_IsRelativePointInside_NonConvex_FarOutside_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::IsRelativePointInside::NonConvex_FarOutside_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Point well clear of the prism's AABB — ray cast must produce zero crossings.
	const FNRawMesh Mesh = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();

	CHECK_FALSE_MESSAGE(TEXT("Point far from the prism must report outside"),
		FNRawMeshUtils::IsRelativePointInside(Mesh, FVector(100, 100, 100)));
}

N_TEST_HIGH(FNRawMeshUtilsTests_AnyRelativePointsInside_NonConvex_BodyAndNotch_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::AnyRelativePointsInside::NonConvex_BodyAndNotch_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mixed input: a notch point (outside) followed by a body point (inside). Verifies that the
	// short-circuit doesn't stop at the first false answer and that AnyRelativePointsInside
	// reaches the parity-raycast path for the non-convex input.
	const FNRawMesh Mesh = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const TArray<FVector> Points = { FVector(5, 8, 2.5), FVector(3, 3, 2.5) };

	CHECK_MESSAGE(TEXT("AnyRelativePointsInside must return true once a body point is found, even on a non-convex mesh"),
		FNRawMeshUtils::AnyRelativePointsInside(Mesh, Points));
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_DoesIntersect_NonConvex_BoxContainedInBody_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::NonConvex_BoxContainedInBody_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The bug DoesIntersect's non-convex fallback fixes: a small cube fully contained inside the
	// non-convex prism's body. No triangle pair crosses, AABBs overlap, and the containment probe
	// must call into the parity ray cast (because the outer mesh is non-convex) to find that the
	// sample vertex is enclosed. Pre-fix this returned false silently with a "not convex" warning.
	const FNRawMesh Prism = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const FNRawMesh SmallCube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(0.5);

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Prism,     FVector::ZeroVector,        FRotator::ZeroRotator,
		SmallCube, FVector(3, 3, 2.5),         FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Small cube fully inside the non-convex prism's body must be reported as intersecting"), bResult);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_DoesIntersect_NonConvex_BoxContainedInNotch_False,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::NonConvex_BoxContainedInNotch_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Same prism, but the small cube sits inside the AABB's notched-out region — geometrically
	// outside the prism. AABBs still overlap so the early-out doesn't fire; the triangle pairs
	// don't cross either (the cube is small enough to stay clear of the notch walls); and the
	// non-convex containment probe must correctly report "outside" via even-parity ray cast.
	// This is the dual of the body-contained test — same code path, different parity.
	const FNRawMesh Prism = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const FNRawMesh SmallCube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(0.25);

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		Prism,     FVector::ZeroVector,        FRotator::ZeroRotator,
		SmallCube, FVector(5, 8, 2.5),         FRotator::ZeroRotator);

	CHECK_FALSE_MESSAGE(TEXT("Small cube parked in the prism's notch must be reported as non-intersecting"), bResult);
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

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_FromChaosBoxParity_MatchesMakeCube,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::FromChaosBoxParity_MatchesMakeCube",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mirrors PartialOverlap_NearestFaceDistance but uses factory-built boxes — proves the FromChaosBox emission
	// path (which populates FaceLoops and flags bIsChaosGenerated) flows through GetIntersectDepth identically to
	// the hand-built MakeCube path. Half-extents 10 and 5 with a +12 X offset → analytic depth = 3.
	const FNRawMesh Left  = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(20.0, 20.0, 20.0);
	const FNRawMesh Right = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(10.0, 10.0, 10.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Left,  FVector(0,  0, 0), FRotator::ZeroRotator,
		Right, FVector(12, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Factory-built boxes must produce the same nearest-face depth (3.0) as the MakeCube case"),
		FMath::IsNearlyEqual(Depth, 3.0f, 0.001f));
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_FromChaosSphere_Concentric_PositiveBounded,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::FromChaosSphere_Concentric_PositiveBounded",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Small sphere (R=2) concentric inside a large sphere (R=10). The cube tests can't exercise non-axis-aligned
	// face normals; this case forces ComputePointDepthInsideConvex to walk hundreds of differently-oriented
	// tessellation planes. For an inner vertex at radius 2 the closest outer triangle plane sits at perp distance
	// R_big * cos(half-segment-angle) - 2, so the max-over-vertices depth ≈ 9.808 (cylinder-band-style faces win).
	const FNRawMesh Big   = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosSphere(10.0);
	const FNRawMesh Small = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosSphere(2.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Big,   FVector(0, 0, 0), FRotator::ZeroRotator,
		Small, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Concentric inner sphere must report a positive depth in the tessellation-bounded range"),
		Depth > 7.0f && Depth < 10.0f);
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_FromChaosSphyl_Contained_PositiveBounded,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::FromChaosSphyl_Contained_PositiveBounded",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Small sphyl (R=2, L=4) co-axial inside a large sphyl (R=10, L=20). Exercises the heterogeneous-face-normal
	// path (pole fans + hemisphere bands + cylinder quads-as-triangles), which the cube tests don't reach. Worst-
	// case depth is bounded by the radial gap minus the small sphyl's outer extent on the cylinder band.
	const FNRawMesh Big   = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosSphyl(10.0, 20.0);
	const FNRawMesh Small = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosSphyl(2.0, 4.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Big,   FVector(0, 0, 0), FRotator::ZeroRotator,
		Small, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Contained sphyl must report a positive depth bounded by the large sphyl's tessellated radial gap"),
		Depth > 7.0f && Depth < 10.0f);
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_FromChaosSphyl_VsBox_SymmetricSwap,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::FromChaosSphyl_VsBox_SymmetricSwap",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Asymmetric-shape symmetry: SymmetricSwap_SameResult only covers identical-shape pairs. A sphyl + box pair
	// has very different face-normal distributions in each direction, so swap-invariance here is a stronger check
	// that the "max of two directional measurements" metric doesn't accidentally short-circuit on the first pass.
	const FNRawMesh Sphyl = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosSphyl(5.0, 10.0);
	const FNRawMesh Box   = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(20.0, 20.0, 20.0);

	const float DepthA = FNRawMeshUtils::GetIntersectDepth(
		Sphyl, FVector(0, 0, 0), FRotator::ZeroRotator,
		Box,   FVector(0, 0, 0), FRotator::ZeroRotator);
	const float DepthB = FNRawMeshUtils::GetIntersectDepth(
		Box,   FVector(0, 0, 0), FRotator::ZeroRotator,
		Sphyl, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Sphyl-vs-Box depth must be identical when the arguments are swapped"),
		FMath::IsNearlyEqual(DepthA, DepthB, 0.001f));
	CHECK_MESSAGE(TEXT("Sphyl co-located with a larger box must report a positive depth"), DepthA > 0.0f);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_FromChaosSphyl_RotatedVsBox_TipPenetration,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::FromChaosSphyl_RotatedVsBox_TipPenetration",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Sphyl (R=2, L=10) rotated 90° about Y so its long axis lies along world -X, then placed at world X=16. Its
	// top pole at local (0,0,7) maps to world (-7,0,0) → translated (9,0,0), which sits 1 unit inside the box's
	// +X face. Verifies that GetIntersectDepth's per-vertex rotation step (LeftQuat / RightQuat in the
	// vertex-sampling loops) is wired correctly — a regression that drops the rotation would either misplace the
	// sphyl entirely (Depth ≈ -1 from AABB miss, or +7 from the tip flush against the box face) or measure depth
	// against the wrong face.
	const FNRawMesh Sphyl = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosSphyl(2.0, 10.0);
	const FNRawMesh Box   = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeChaosBox(20.0, 20.0, 20.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Box,   FVector(0,  0, 0), FRotator::ZeroRotator,
		Sphyl, FVector(16, 0, 0), FRotator(90.f, 0.f, 0.f));

	CHECK_MESSAGE(TEXT("Rotated sphyl tip 1 unit inside the box's +X face must report depth ≈ 1"),
		Depth > 0.5f && Depth < 1.5f);
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_SurfaceOnlyCrossing_ReturnsZero,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::SurfaceOnlyCrossing_ReturnsZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two perpendicular thin rods crossing at the origin: their bodies overlap in the middle but every vertex of
	// each rod sits well outside the other's cross-section (|x|=100 vs cross-section |x|≤1, and vice-versa).
	// Locks the documented contract "Returns 0.0 when the AABBs overlap but no vertex of either mesh lies inside
	// the other" — a bug that double-counted boundary-touching vertices or accidentally returned -1 here would
	// silently corrupt threshold checks built on this API.
	const FNRawMesh RodX = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeBox(100.0, 1.0, 1.0);
	const FNRawMesh RodY = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeBox(1.0, 100.0, 1.0);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		RodX, FVector(0, 0, 0), FRotator::ZeroRotator,
		RodY, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Surface-only crossing without vertex containment must return exactly 0, not -1 and not positive"),
		FMath::IsNearlyEqual(Depth, 0.0f, 0.001f));
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_NonConvex_VertexInBody_ReturnsPositiveDepth,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::NonConvex_VertexInBody_ReturnsPositiveDepth",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Small unit cube parked inside the body of the non-convex prism at (3, 3, 2.5). All 8 cube
	// vertices clear the prism's footprint (the L-body covers (3, 3)) and sit at z ∈ [2.0, 3.0],
	// equidistant from the z=0 and z=5 caps — nearest surface is one of the top/bottom faces at
	// distance 2.0. This exercises the full non-convex path: IsRelativePointInside parity probe
	// per vertex + point-to-triangle distance.
	const FNRawMesh Prism = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const FNRawMesh SmallCube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(0.5);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Prism,     FVector::ZeroVector,    FRotator::ZeroRotator,
		SmallCube, FVector(3, 3, 2.5),     FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Cube fully inside the non-convex prism's body must report a positive depth"),
		Depth > 0.0f);
	CHECK_MESSAGE(TEXT("Depth should be ~2.0 — distance from each cube vertex (z ∈ [2.0, 3.0]) to the prism's nearest cap"),
		FMath::IsNearlyEqual(Depth, 2.0f, 0.05f));
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_GetIntersectDepth_NonConvex_VertexInNotch_ReturnsZero,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::NonConvex_VertexInNotch_ReturnsZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Small cube parked inside the prism's AABB but in the cut-out notch — no cube vertex is
	// inside the prism's body, and no prism vertex is inside the cube. Result must be exactly 0,
	// matching the existing "AABBs overlap, no vertex containment" semantic for surface-only
	// crossings on convex meshes.
	const FNRawMesh Prism = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const FNRawMesh SmallCube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(0.25);

	const float Depth = FNRawMeshUtils::GetIntersectDepth(
		Prism,     FVector::ZeroVector,    FRotator::ZeroRotator,
		SmallCube, FVector(5, 8, 2.5),     FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Cube parked in the prism's notch (no vertex inside the body) must return exactly 0"),
		FMath::IsNearlyEqual(Depth, 0.0f, 0.001f));
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_NonConvex_DeeperPenetration_HigherDepth,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::NonConvex_DeeperPenetration_HigherDepth",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Sanity check on the monotonicity of the metric: a cube nudged close to the top cap (z=4.5)
	// must report a smaller depth than the same cube parked at the prism's centre. Catches "we
	// measured to the wrong wall" bugs that the convex algorithm's plane test would mask, since
	// for a non-convex mesh the right answer is distance-to-nearest-surface, not the cube's own
	// offset from any single face plane.
	const FNRawMesh Prism = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const FNRawMesh SmallCube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(0.25);

	const float ShallowDepth = FNRawMeshUtils::GetIntersectDepth(
		Prism,     FVector::ZeroVector,    FRotator::ZeroRotator,
		SmallCube, FVector(3, 3, 4.5),     FRotator::ZeroRotator);

	const float CentredDepth = FNRawMeshUtils::GetIntersectDepth(
		Prism,     FVector::ZeroVector,    FRotator::ZeroRotator,
		SmallCube, FVector(3, 3, 2.5),     FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Cube parked near the top of the prism must report a smaller depth than one in the centre"),
		ShallowDepth < CentredDepth);
	CHECK_MESSAGE(TEXT("Shallow penetration should still be positive — every cube vertex is inside the body"),
		ShallowDepth > 0.0f);
}

N_TEST_HIGH(FNRawMeshUtilsTests_GetIntersectDepth_NonConvex_SymmetricSwap_SameResult,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::GetIntersectDepth::NonConvex_SymmetricSwap_SameResult",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The metric is symmetric by construction — swapping (Left, Right) must yield bitwise the same
	// result. Verifies the new dispatch behaves the same regardless of which mesh is the convex one
	// and which is non-convex (each calls ComputePointDepthInside on the opposite mesh).
	const FNRawMesh Prism = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeNonConvexPrism();
	const FNRawMesh SmallCube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(0.5);

	const float DepthLeft = FNRawMeshUtils::GetIntersectDepth(
		Prism,     FVector::ZeroVector,    FRotator::ZeroRotator,
		SmallCube, FVector(3, 3, 2.5),     FRotator::ZeroRotator);

	const float DepthRight = FNRawMeshUtils::GetIntersectDepth(
		SmallCube, FVector(3, 3, 2.5),     FRotator::ZeroRotator,
		Prism,     FVector::ZeroVector,    FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("GetIntersectDepth must be argument-symmetric for non-convex inputs too"),
		FMath::IsNearlyEqual(DepthLeft, DepthRight, 0.001f));
}

N_TEST_HIGH(FNRawMeshUtilsTests_DoesIntersect_SurfaceOnlyCrossing_True,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::DoesIntersect::SurfaceOnlyCrossing_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Paired with GetIntersectDepth::SurfaceOnlyCrossing_ReturnsZero — same geometry, but DoesIntersect must
	// return true (the rods do geometrically overlap via triangle-vs-triangle) while the depth metric
	// returns 0 (no vertex of either lies inside the other). Locks in the contract that the two APIs
	// measure different things and can legitimately disagree on the same input.
	const FNRawMesh RodX = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeBox(100.0, 1.0, 1.0);
	const FNRawMesh RodY = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeBox(1.0, 100.0, 1.0);

	const bool bResult = FNRawMeshUtils::DoesIntersect(
		RodX, FVector(0, 0, 0), FRotator::ZeroRotator,
		RodY, FVector(0, 0, 0), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Perpendicular rods pass through each other geometrically — DoesIntersect must return true"), bResult);
}

N_TEST_HIGH(FNRawMeshUtilsTests_ToConvexHull_AlreadyConvex_ReturnsCopyWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::ToConvexHull::AlreadyConvex_ReturnsCopyWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("already a convex hull"), ELogVerbosity::Warning);

	// Validated convex cube must early-return as a duplicate rather than being re-built — the warning is the
	// audible cue to the caller that they have a redundant call site.
	const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
	CHECK_MESSAGE(TEXT("Input cube must report convex going in"), Cube.IsConvex());

	const FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Cube);

	CHECK_MESSAGE(TEXT("Already-convex input must be returned as an equal copy"), Hull == Cube);
}

N_TEST_HIGH(FNRawMeshUtilsTests_ToConvexHull_TooFewVertices_ReturnsEmptyWithWarning,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::ToConvexHull::TooFewVertices_ReturnsEmptyWithWarning",
	N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("fewer than 4 vertices"), ELogVerbosity::Warning);

	// A single triangle cannot form a 3D hull. Chaos's builder needs >=4 points; the guard short-circuits
	// before reaching it and returns an empty mesh so the caller's downstream pipeline can detect the failure.
	FNRawMesh Input;
	Input.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };

	const FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Input);

	CHECK_EQUALS("Too-few-vertices input must yield an empty hull (no vertices)", Hull.Vertices.Num(), 0);
	CHECK_EQUALS("Too-few-vertices input must yield zero loops", Hull.Loops.Num(), 0);
	CHECK_EQUALS("Too-few-vertices input must yield zero FaceLoops", Hull.FaceLoops.Num(), 0);
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_ToConvexHull_CubeCloud_BuildsChaosHullWithTriangulatedFaces,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::ToConvexHull::CubeCloud_BuildsChaosHullWithTriangulatedFaces",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Feeding 8 cube vertices (no triangles, no Validate so bIsConvex stays false and the early-out doesn't
	// fire) drives the actual Chaos hull builder. ToConvexHull hard-codes EBuildMethod::Original
	// (NRawMeshUtils.cpp:245), which emits already-triangulated face indices — so for a cube the result is
	// 12 triangle FaceLoops, not 6 quads (contrast FromChaosConvexHull, which uses the default builder and
	// preserves quads). The "polygonal pre-triangulation" intent of FaceLoops is effectively a no-op on this
	// path; this test documents that contract.
	FNRawMesh Cloud;
	Cloud.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
	};

	const FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Cloud);

	CHECK_EQUALS("Cube cloud should produce 8 hull vertices", Hull.Vertices.Num(), 8);
	CHECK_EQUALS("EBuildMethod::Original pre-triangulates each cube face → 12 triangle FaceLoops", Hull.FaceLoops.Num(), 12);
	for (int32 i = 0; i < Hull.FaceLoops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Each cube-hull FaceLoop should be a triangle on the Original-builder path"), Hull.FaceLoops[i].IsTriangle());
	}
	CHECK_MESSAGE(TEXT("Hull should report convex via the cached flag"), Hull.IsConvex());
	CHECK_MESSAGE(TEXT("Hull should have populated bounds"), Hull.HasBounds());
	CHECK_FALSE_MESSAGE(TEXT("Loops should be all-triangles after the builder + ConvertToTriangles pass"), Hull.HasNonTris());
}

N_TEST_HIGH(FNRawMeshUtilsTests_ToConvexHull_SpikeCubeCloud_IncludesSpikeAsHullVertex,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::ToConvexHull::SpikeCubeCloud_IncludesSpikeAsHullVertex",
	N_TEST_CONTEXT_ANYWHERE)
{
	// 8 cube corners + 1 spike well above the cube. The spike is an extreme point in +Z and every cube
	// corner remains extreme in its own ±X/±Y direction, so the hull must contain all 9 input points.
	FNRawMesh Cloud;
	Cloud.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
		{  0,  0, 100 },
	};

	const FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Cloud);

	CHECK_EQUALS("Spike-cube cloud should produce a 9-vertex hull (all inputs are extreme points)", Hull.Vertices.Num(), 9);
	CHECK_MESSAGE(TEXT("Spike-cube hull must report convex by construction"), Hull.IsConvex());
	CHECK_MESSAGE(TEXT("Spike-cube hull Z extent should reach the spike at z=100"),
		FMath::IsNearlyEqual(Hull.Bounds.Max.Z, 100.0, 0.001));
}

N_TEST_HIGH(FNRawMeshUtilsTests_CombineMesh_ClearsChaosGeneratedFlag,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CombineMesh::ClearsChaosGeneratedFlag",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Header @note on CombineMesh: "BaseMesh's bIsChaosGenerated flag is cleared since the merged result is
	// no longer a single cooked Chaos body." Observable side effect: after the merge, the internal Validate
	// must re-evaluate convexity rather than skip. Test by combining two convex hulls 100 units apart — if
	// the flag weren't cleared, Validate would skip and IsConvex would inherit Base's incoming true; with the
	// flag cleared, Validate correctly detects the disjoint-cube merge as non-convex.
	FNRawMesh CloudA;
	CloudA.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
	};
	FNRawMesh HullA = FNRawMeshUtils::ToConvexHull(CloudA);
	CHECK_MESSAGE(TEXT("HullA must be Chaos-flagged and convex before the merge"), HullA.IsConvex());

	FNRawMesh CloudB;
	CloudB.Vertices = {
		{ +95, -5, -5 }, { +105, -5, -5 }, { +105, +5, -5 }, { +95, +5, -5 },
		{ +95, -5, +5 }, { +105, -5, +5 }, { +105, +5, +5 }, { +95, +5, +5 },
	};
	const FNRawMesh HullB = FNRawMeshUtils::ToConvexHull(CloudB);
	CHECK_MESSAGE(TEXT("HullB must also be Chaos-flagged and convex before the merge"), HullB.IsConvex());

	FNRawMeshUtils::CombineMesh(FTransform::Identity, HullA, FTransform::Identity, HullB);

	CHECK_FALSE_MESSAGE(TEXT("Two convex hulls 100 units apart must merge to a non-convex result — proves CombineMesh cleared bIsChaosGenerated so the internal Validate ran"),
		HullA.IsConvex());
}

N_TEST_HIGH(FNRawMeshUtilsTests_CreateRawMeshVisualizers_PerMesh_SpawnsOnePerEntry,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CreateRawMeshVisualizers::PerMesh_SpawnsOnePerEntry",
	N_TEST_CONTEXT_EDITOR)
{
	// Per-mesh mode: one ANDebugActor per (Mesh, Transform) pair, each spawned at its matching transform
	// and marked transient so the actors don't leak into the saved level.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
		const TArray<FNRawMesh> Meshes = { Cube, Cube, Cube };
		const TArray<FTransform> Transforms = {
			FTransform(FVector(0,   0, 0)),
			FTransform(FVector(50,  0, 0)),
			FTransform(FVector(100, 0, 0)),
		};

		const TArray<ANDebugActor*> Actors = FNRawMeshUtils::CreateRawMeshVisualizers(
			World, Meshes, Transforms, nullptr, /*bSingleActor*/ false, /*bProcessMeshes*/ false);

		CHECK_EQUALS("Per-mesh mode must spawn one actor per Meshes/Transforms entry", Actors.Num(), 3);
		for (int32 i = 0; i < Actors.Num(); i++)
		{
			if (Actors[i] == nullptr)
			{
				ADD_ERROR("Spawned visualizer actor was unexpectedly null");
				continue;
			}
			CHECK_MESSAGE(TEXT("Each visualizer must be an ANDebugActor"), Actors[i]->IsA<ANDebugActor>());
			CHECK_MESSAGE(TEXT("Each visualizer must be RF_Transient so it does not persist with the level"),
				Actors[i]->HasAnyFlags(RF_Transient));
			CHECK_MESSAGE(TEXT("Each visualizer must land at its source transform"),
				Actors[i]->GetActorLocation().Equals(Transforms[i].GetLocation(), 0.001));
		}
	});
}

N_TEST_HIGH(FNRawMeshUtilsTests_CreateRawMeshVisualizers_SingleActor_SpawnsOneAtIdentity,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::CreateRawMeshVisualizers::SingleActor_SpawnsOneAtIdentity",
	N_TEST_CONTEXT_EDITOR)
{
	// Single-actor mode: all entries are merged via CombineMesh into one ANDebugActor anchored at identity.
	// The per-entry transforms are used to align the meshes during the merge, not to position the actor.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		const FNRawMesh Cube = NEXUS::UnitTests::NCore::FNRawMeshUtilsHarness::MakeCube(5.0);
		const TArray<FNRawMesh> Meshes = { Cube, Cube };
		const TArray<FTransform> Transforms = {
			FTransform(FVector(0,  0, 0)),
			FTransform(FVector(50, 0, 0)),
		};

		const TArray<ANDebugActor*> Actors = FNRawMeshUtils::CreateRawMeshVisualizers(
			World, Meshes, Transforms, nullptr, /*bSingleActor*/ true, /*bProcessMeshes*/ false);

		CHECK_EQUALS("Single-actor mode must spawn exactly one actor regardless of input count", Actors.Num(), 1);
		if (Actors.Num() != 1 || Actors[0] == nullptr)
		{
			ADD_ERROR("Single-actor spawn produced no usable actor to assert on");
			return;
		}
		CHECK_MESSAGE(TEXT("Single-actor visualizer must be an ANDebugActor"), Actors[0]->IsA<ANDebugActor>());
		CHECK_MESSAGE(TEXT("Single-actor visualizer must spawn at the merged-mesh base transform (identity)"),
			Actors[0]->GetActorLocation().Equals(FVector::ZeroVector, 0.001));
	});
}

#endif //WITH_TESTS
