// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNRawMeshTests_GetFlatIndices_Empty, "NEXUS::UnitTests::NCore::FNRawMesh::GetFlatIndices::Empty", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	CHECK_EQUALS("Empty mesh should produce an empty index array", Mesh.GetFlatIndices().Num(), 0);
}

N_TEST_HIGH(FNRawMeshTests_GetFlatIndices_Mixed, "NEXUS::UnitTests::NCore::FNRawMesh::GetFlatIndices::Mixed", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(1, 1, 0), FVector(0, 1, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));

	const TArray<int32> Flat = Mesh.GetFlatIndices();
	CHECK_EQUALS("Flat index count should equal the sum of every loop's index count (3 + 4)", Flat.Num(), 7);
	CHECK_EQUALS("Flat[0] should mirror the first loop's first index", Flat[0], 0);
	CHECK_EQUALS("Flat[3] should mirror the second loop's first index", Flat[3], 0);
	CHECK_EQUALS("Flat[6] should mirror the second loop's last index", Flat[6], 3);
}

N_TEST_CRITICAL(FNRawMeshTests_ConvertToTriangles_TriangleOnly_NoOp, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::TriangleOnly_NoOp", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Triangle-only mesh should remain a single loop after ConvertToTriangles", Mesh.Loops.Num(), 1);
	CHECK_MESSAGE(TEXT("Remaining loop should still be a triangle"), Mesh.Loops[0].IsTriangle());
}

N_TEST_CRITICAL(FNRawMeshTests_ConvertToTriangles_Quad_FanPath, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::Quad_FanPath", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_MESSAGE(TEXT("Axis-aligned convex quad should report IsConvex==true so the fan path runs"), Mesh.IsConvex());

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Quad should triangulate into exactly 2 triangles", Mesh.Loops.Num(), 2);
	CHECK_MESSAGE(TEXT("First emitted loop should be a triangle"), Mesh.Loops[0].IsTriangle());
	CHECK_MESSAGE(TEXT("Second emitted loop should be a triangle"), Mesh.Loops[1].IsTriangle());

	// Fan-from-0 should emit (0,1,2) followed by (0,2,3).
	CHECK_EQUALS("Fan triangle #1 vertex 0", Mesh.Loops[0].Indices[0], 0);
	CHECK_EQUALS("Fan triangle #1 vertex 1", Mesh.Loops[0].Indices[1], 1);
	CHECK_EQUALS("Fan triangle #1 vertex 2", Mesh.Loops[0].Indices[2], 2);
	CHECK_EQUALS("Fan triangle #2 vertex 0", Mesh.Loops[1].Indices[0], 0);
	CHECK_EQUALS("Fan triangle #2 vertex 1", Mesh.Loops[1].Indices[1], 2);
	CHECK_EQUALS("Fan triangle #2 vertex 2", Mesh.Loops[1].Indices[2], 3);
}

N_TEST_HIGH(FNRawMeshTests_ConvertToTriangles_ConvexPentagon_FanPath, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::ConvexPentagon_FanPath", N_TEST_CONTEXT_ANYWHERE)
{
	// Geometrically convex pentagon — the in-plane convexity test should flag it convex and the
	// fan-from-0 branch should run, emitting (0,1,2), (0,2,3), (0,3,4).
	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(0,   0,  0),
		FVector(10,  0,  0),
		FVector(13,  8,  0),
		FVector(5,   14, 0),
		FVector(-3,  8,  0)
	};
	Mesh.Loops.Add(FNRawMeshLoop(TArray<int32>{0, 1, 2, 3, 4}));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_MESSAGE(TEXT("Convex pentagon should report IsConvex==true under the plane-side convexity test"), Mesh.IsConvex());

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Pentagon should triangulate into n-2 = 3 triangles", Mesh.Loops.Num(), 3);
	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop should be a triangle"), Mesh.Loops[i].IsTriangle());
	}

	// Fan-from-0 guarantees every triangle starts with Src[0].
	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		CHECK_EQUALS("Fan-from-0 triangles must all share vertex index 0 as their first vertex", Mesh.Loops[i].Indices[0], 0);
	}
}

N_TEST_HIGH(FNRawMeshTests_Validate_IsConvex_ConcaveNgon, "NEXUS::UnitTests::NCore::FNRawMesh::Validate::IsConvex_ConcaveNgon", N_TEST_CONTEXT_ANYWHERE)
{
	// Concave arrow-with-notch pentagon — the in-plane convexity check must flag vertex 3 as
	// turning the wrong way relative to the face normal.
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

	CHECK_FALSE_MESSAGE(TEXT("Concave n-gon must report IsConvex==false"), Mesh.IsConvex());
}

N_TEST_CRITICAL(FNRawMeshTests_ConvertToTriangles_ConcavePentagon_CorrectArea, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::ConcavePentagon_CorrectArea", N_TEST_CONTEXT_ANYWHERE)
{
	// Concave pentagon in the XY plane — an arrow with a notch on the left side. Shoelace
	// area = 36. Fan-from-0 would overlap, producing a larger total absolute area; correct
	// ear-clipping must produce non-overlapping triangles whose areas sum to the polygon area.
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

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Concave pentagon should triangulate into 3 triangles", Mesh.Loops.Num(), 3);

	double TotalArea = 0.0;
	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		const FVector& A = Mesh.Vertices[Mesh.Loops[i].Indices[0]];
		const FVector& B = Mesh.Vertices[Mesh.Loops[i].Indices[1]];
		const FVector& C = Mesh.Vertices[Mesh.Loops[i].Indices[2]];
		const double Cross = (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
		TotalArea += FMath::Abs(Cross) * 0.5;
	}

	CHECK_MESSAGE(TEXT("Sum of absolute triangle areas must match polygon area (36.0) — any overlap indicates a broken triangulation"),
		FMath::IsNearlyEqual(TotalArea, 36.0, 0.001));
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_Identical, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::Identical", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));
	A.CalculateCenterAndBounds();
	A.Validate();

	FNRawMesh B = A;
	CHECK_MESSAGE(TEXT("Identical meshes should compare equal"), A == B);
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_DifferentVertices, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::DifferentVertices", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));
	A.CalculateCenterAndBounds();
	A.Validate();

	FNRawMesh B = A;
	B.Vertices[1] = FVector(2, 0, 0);
	CHECK_FALSE_MESSAGE(TEXT("Meshes with different vertex positions should not compare equal"), A == B);
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_DifferentLoopCount, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::DifferentLoopCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0), FVector(1, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));

	FNRawMesh B = A;
	B.Loops.Add(FNRawMeshLoop(1, 3, 2));
	CHECK_FALSE_MESSAGE(TEXT("Meshes with differing loop counts should not compare equal"), A == B);
}

N_TEST_HIGH(FNRawMeshTests_CalculateCenterAndBounds_SymmetricCube, "NEXUS::UnitTests::NCore::FNRawMesh::CalculateCenterAndBounds::SymmetricCube", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(-10, -10, -10),
		FVector( 10, -10, -10),
		FVector( 10,  10, -10),
		FVector(-10,  10, -10),
		FVector(-10, -10,  10),
		FVector( 10, -10,  10),
		FVector( 10,  10,  10),
		FVector(-10,  10,  10)
	};

	Mesh.CalculateCenterAndBounds();

	CHECK_MESSAGE(TEXT("Center of a symmetric cube should be the origin"), Mesh.Center.Equals(FVector::ZeroVector));
	CHECK_MESSAGE(TEXT("Bounds min should be (-10,-10,-10)"), Mesh.Bounds.Min.Equals(FVector(-10, -10, -10)));
	CHECK_MESSAGE(TEXT("Bounds max should be (10,10,10)"), Mesh.Bounds.Max.Equals(FVector(10, 10, 10)));
	CHECK_MESSAGE(TEXT("CalculateCenterAndBounds should mark HasBounds() as true"), Mesh.HasBounds());
}

N_TEST_HIGH(FNRawMeshTests_Validate_HasNonTris_Quad, "NEXUS::UnitTests::NCore::FNRawMesh::Validate::HasNonTris_Quad", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(1, 1, 0), FVector(0, 1, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_MESSAGE(TEXT("Mesh with any non-triangle loop should report HasNonTris()==true"), Mesh.HasNonTris());
}

N_TEST_HIGH(FNRawMeshTests_Validate_HasNonTris_TrianglesOnly, "NEXUS::UnitTests::NCore::FNRawMesh::Validate::HasNonTris_TrianglesOnly", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Triangle-only mesh should report HasNonTris()==false"), Mesh.HasNonTris());
}

N_TEST_HIGH(FNRawMeshTests_RotatedAroundPivot_ZeroRotation, "NEXUS::UnitTests::NCore::FNRawMesh::RotatedAroundPivot::ZeroRotation", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Center = FVector(5, 5, 0);

	Mesh.RotatedAroundPivot(FVector::ZeroVector, FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("Zero rotation around origin should leave vertex 0 unchanged"), Mesh.Vertices[0].Equals(FVector(10, 0, 0)));
	CHECK_MESSAGE(TEXT("Zero rotation around origin should leave vertex 1 unchanged"), Mesh.Vertices[1].Equals(FVector(0, 10, 0)));
	CHECK_MESSAGE(TEXT("Zero rotation around origin should leave center unchanged"), Mesh.Center.Equals(FVector(5, 5, 0)));
}

N_TEST_HIGH(FNRawMeshTests_RotatedAroundPivot_Yaw180, "NEXUS::UnitTests::NCore::FNRawMesh::RotatedAroundPivot::Yaw180", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(10, 0, 0) };
	Mesh.Center = FVector::ZeroVector;

	Mesh.RotatedAroundPivot(FVector::ZeroVector, FRotator(0.f, 180.f, 0.f));

	CHECK_MESSAGE(TEXT("180-degree yaw around the origin should negate X"),
		Mesh.Vertices[0].Equals(FVector(-10, 0, 0), 0.01));
}

N_TEST_HIGH(FNRawMeshTests_GetFlatIndices_SingleTriangle, "NEXUS::UnitTests::NCore::FNRawMesh::GetFlatIndices::SingleTriangle", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	const TArray<int32> Flat = Mesh.GetFlatIndices();
	CHECK_EQUALS("Single triangle should flatten into three indices", Flat.Num(), 3);
	CHECK_EQUALS("Flat[0] should equal the triangle's first index", Flat[0], 0);
	CHECK_EQUALS("Flat[1] should equal the triangle's second index", Flat[1], 1);
	CHECK_EQUALS("Flat[2] should equal the triangle's third index", Flat[2], 2);
}

N_TEST_HIGH(FNRawMeshTests_ConvertToTriangles_Empty_NoOp, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::Empty_NoOp", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Empty mesh should remain empty after ConvertToTriangles", Mesh.Loops.Num(), 0);
	CHECK_EQUALS("Empty mesh should have no vertices introduced", Mesh.Vertices.Num(), 0);
}

N_TEST_HIGH(FNRawMeshTests_ConvertToTriangles_MixedTriAndQuad_PreservesTri, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::MixedTriAndQuad_PreservesTri", N_TEST_CONTEXT_ANYWHERE)
{
	// A triangle and a quad in the same mesh — the triangle must pass through untouched while the quad expands to two tris.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(0, 0, 0), FVector(10, 0, 0), FVector(5, 10, 0),
		FVector(20, 0, 0), FVector(30, 0, 0), FVector(30, 10, 0), FVector(20, 10, 0)
	};
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(3, 4, 5, 6));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Mixed input should produce 1 preserved tri + 2 quad-split tris = 3 total", Mesh.Loops.Num(), 3);
	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every remaining loop should be a triangle"), Mesh.Loops[i].IsTriangle());
	}
	CHECK_EQUALS("Preserved triangle's first vertex should be unchanged", Mesh.Loops[0].Indices[0], 0);
	CHECK_EQUALS("Preserved triangle's second vertex should be unchanged", Mesh.Loops[0].Indices[1], 1);
	CHECK_EQUALS("Preserved triangle's third vertex should be unchanged", Mesh.Loops[0].Indices[2], 2);
}

N_TEST_CRITICAL(FNRawMeshTests_ConvertToTriangles_ConcaveQuad_EarClipping, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::ConcaveQuad_EarClipping", N_TEST_CONTEXT_ANYWHERE)
{
	// Dart-shaped concave quad — shoelace area = 40. Ear-clipping must emit 2 non-overlapping triangles summing to 40.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(0,  0,  0),
		FVector(10, 0,  0),
		FVector(4,  4,  0),
		FVector(0,  10, 0)
	};
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Dart-shaped quad must report IsConvex==false so the ear-clipping branch runs"), Mesh.IsConvex());

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Concave quad should triangulate into n-2 = 2 triangles", Mesh.Loops.Num(), 2);

	double TotalArea = 0.0;
	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		const FVector& A = Mesh.Vertices[Mesh.Loops[i].Indices[0]];
		const FVector& B = Mesh.Vertices[Mesh.Loops[i].Indices[1]];
		const FVector& C = Mesh.Vertices[Mesh.Loops[i].Indices[2]];
		const double Cross = (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
		TotalArea += FMath::Abs(Cross) * 0.5;
	}

	CHECK_MESSAGE(TEXT("Triangle areas must sum to the dart's shoelace area (40.0) — overlap would indicate a broken ear clip"),
		FMath::IsNearlyEqual(TotalArea, 40.0, 0.001));
}

N_TEST_HIGH(FNRawMeshTests_ConvertToTriangles_Collinear_FallbackWarning, "NEXUS::UnitTests::NCore::FNRawMesh::ConvertToTriangles::Collinear_FallbackWarning", N_TEST_CONTEXT_ANYWHERE)
{
	// Four collinear vertices form a degenerate loop — ear-clipping can never find a convex ear, so the code must
	// warn and drop the unresolved remainder rather than fanning it into self-intersecting / zero-area triangles.
	AddExpectedMessage(TEXT("Ear-clipping could not fully triangulate loop"), ELogVerbosity::Warning);

	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(2, 0, 0), FVector(3, 0, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	// Deliberately skip Validate() so bIsConvex stays false and the ear-clipping branch runs on this degenerate input.

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Degenerate collinear quad must drop the unresolved remainder rather than emitting bad triangles", Mesh.Loops.Num(), 0);
}

N_TEST_HIGH(FNRawMeshTests_CheckConvex_Cube_True, "NEXUS::UnitTests::NCore::FNRawMesh::CheckConvex::Cube_True", N_TEST_CONTEXT_ANYWHERE)
{
	// Axis-aligned cube using the same outward winding as FNRawMeshFactory::FromChaosBox — exercises the 3D plane-side
	// test, which the flat-ngon convexity tests don't cover.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
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

	CHECK_MESSAGE(TEXT("Axis-aligned cube must pass both the in-plane convexity test and the plane-side test"), Mesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_CheckConvex_ConcavePolyhedron_False, "NEXUS::UnitTests::NCore::FNRawMesh::CheckConvex::ConcavePolyhedron_False", N_TEST_CONTEXT_ANYWHERE)
{
	// Cube augmented with a "spike" vertex that is not part of any face but sits far outside the top face's
	// supporting plane — in-plane convexity passes (every face is a triangle), but the plane-side test must reject it.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
		{  0,  0, 100 },
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

	CHECK_FALSE_MESSAGE(TEXT("Spike vertex sits outside a face's plane so the polyhedron must report non-convex"), Mesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_CheckConvex_Empty_FalseWithWarning, "NEXUS::UnitTests::NCore::FNRawMesh::CheckConvex::Empty_FalseWithWarning", N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("No vertices or loops were found in the FNRawMesh"), ELogVerbosity::Warning);

	FNRawMesh Mesh;
	Mesh.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Empty mesh must report IsConvex==false and log a warning"), Mesh.IsConvex());
}

N_TEST_CRITICAL(FNRawMeshTests_CheckConvex_FaceLoops_FanTriangulatedCubeAfterVertexNudge,
	"NEXUS::UnitTests::NCore::FNRawMesh::CheckConvex::FaceLoops_FanTriangulatedCubeAfterVertexNudge",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Repro for the visualizer regression: a hull built by Chaos arrives as ngonal faces, gets fan-triangulated,
	// and the per-triangle plane test then rejects any subsequent vertex edit because coplanar neighbor triangles
	// drift slightly off each other's plane. With FaceLoops carrying the polygonal source, CheckConvex must walk
	// the n-gons and accept the edit. Coordinates are at editor-realistic scale to make floating-point drift
	// large enough to break the per-triangle path.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -500, -500, -500 }, { +500, -500, -500 }, { +500, +500, -500 }, { -500, +500, -500 },
		{ -500, -500, +500 }, { +500, -500, +500 }, { +500, +500, +500 }, { -500, +500, +500 },
	};
	// Six quad faces wound outward.
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 3, 2, 1)); // -Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(4, 5, 6, 7)); // +Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 1, 5, 4)); // -Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(1, 2, 6, 5)); // +X
	Mesh.FaceLoops.Add(FNRawMeshLoop(2, 3, 7, 6)); // +Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(3, 0, 4, 7)); // -X
	Mesh.Loops = Mesh.FaceLoops;
	Mesh.ConvertToTriangles();
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_MESSAGE(TEXT("Fan-triangulated cube described via FaceLoops should pass CheckConvex out of the gate"), Mesh.IsConvex());

	// Pull one vertex inward by 1 unit — the cube is still convex, but the per-triangle plane test on Loops would reject this.
	Mesh.Vertices[1] = FVector(+499, -499, -500);
	Mesh.Validate();

	CHECK_MESSAGE(TEXT("Nudging a corner inward must still report convex when CheckConvex walks FaceLoops"), Mesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_CheckConvex_PlaneSide_ExtentScaledTolerance,
	"NEXUS::UnitTests::NCore::FNRawMesh::CheckConvex::PlaneSide_ExtentScaledTolerance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The plane-side test must share the extent-scaled planarity budget rather than use an absolute
	// epsilon. A cube with a diagonally split top: raising the corner only one triangle owns keeps
	// every face planar and every loop convex, but puts that corner (and the opposite top corner) a
	// hair in front of the other triangle's plane — sub-tolerance drift that an absolute epsilon
	// false-fails at editor-realistic scale (matching FaceLoops_FanTriangulatedCubeAfterVertexNudge).
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -500, -500, -500 }, { +500, -500, -500 }, { +500, +500, -500 }, { -500, +500, -500 },
		{ -500, -500, +500 }, { +500, -500, +500 }, { +500, +500, +500 }, { -500, +500, +500 },
	};
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 3, 2, 1)); // -Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(4, 5, 6));    // +Z split diagonally so vertex 5
	Mesh.FaceLoops.Add(FNRawMeshLoop(4, 6, 7));    // belongs to only one of the halves
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 1, 5, 4)); // -Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(1, 2, 6, 5)); // +X
	Mesh.FaceLoops.Add(FNRawMeshLoop(2, 3, 7, 6)); // +Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(3, 0, 4, 7)); // -X
	Mesh.Loops = Mesh.FaceLoops;
	Mesh.ConvertToTriangles();
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_MESSAGE(TEXT("Diagonally split cube should pass CheckConvex out of the gate"), Mesh.IsConvex());

	// Raise the corner by 0.4 — well inside the 1.0 (1e-3 x extent) budget at this scale, but four
	// thousand times an absolute UE_DOUBLE_KINDA_SMALL_NUMBER threshold.
	Mesh.Vertices[5].Z = 500.4;
	Mesh.Validate();
	CHECK_MESSAGE(TEXT("Sub-tolerance ridge across the split top must still report convex"), Mesh.IsConvex());

	// Push it past the budget — the scaled threshold must still reject real concavity.
	Mesh.Vertices[5].Z = 505.0;
	Mesh.Validate();
	CHECK_FALSE_MESSAGE(TEXT("Above-tolerance ridge must trip the plane-side test"), Mesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_CheckConvex_FaceLoops_DetectsConcavity,
	"NEXUS::UnitTests::NCore::FNRawMesh::CheckConvex::FaceLoops_DetectsConcavity",
	N_TEST_CONTEXT_ANYWHERE)
{
	// FaceLoops must not silently pass concave geometry. A spike vertex outside the cube's top face plane should
	// be detected by the plane-side test when walking the polygonal faces, exactly like it is on the triangle path.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -500, -500, -500 }, { +500, -500, -500 }, { +500, +500, -500 }, { -500, +500, -500 },
		{ -500, -500, +500 }, { +500, -500, +500 }, { +500, +500, +500 }, { -500, +500, +500 },
		{    0,    0, 5000 }, // spike well above the +Z face
	};
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 3, 2, 1));
	Mesh.FaceLoops.Add(FNRawMeshLoop(4, 5, 6, 7));
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 1, 5, 4));
	Mesh.FaceLoops.Add(FNRawMeshLoop(1, 2, 6, 5));
	Mesh.FaceLoops.Add(FNRawMeshLoop(2, 3, 7, 6));
	Mesh.FaceLoops.Add(FNRawMeshLoop(3, 0, 4, 7));
	Mesh.Loops = Mesh.FaceLoops;
	Mesh.ConvertToTriangles();
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Spike vertex outside the +Z face plane must trip the plane-side test on the FaceLoops path"), Mesh.IsConvex());
}

N_TEST_CRITICAL(FNRawMeshTests_CalculateFaceLoops_TriangulatedCube_SixQuads,
	"NEXUS::UnitTests::NCore::FNRawMesh::CalculateFaceLoops::TriangulatedCube_SixQuads",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Fan-triangulated cube — recovery should merge each pair of coplanar triangles back into a quad and produce
	// exactly 6 faces. Vertex windings were chosen to match FromChaosBox so the resulting normals point outward.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
	};
	static const int32 Tris[12][3] = {
		{ 0, 3, 2 }, { 0, 2, 1 },  // -Z (quad 0,3,2,1)
		{ 4, 5, 6 }, { 4, 6, 7 },  // +Z (quad 4,5,6,7)
		{ 0, 1, 5 }, { 0, 5, 4 },  // -Y (quad 0,1,5,4)
		{ 1, 2, 6 }, { 1, 6, 5 },  // +X (quad 1,2,6,5)
		{ 2, 3, 7 }, { 2, 7, 6 },  // +Y (quad 2,3,7,6)
		{ 3, 0, 4 }, { 3, 4, 7 },  // -X (quad 3,0,4,7)
	};
	for (int32 i = 0; i < 12; ++i)
	{
		Mesh.Loops.Add(FNRawMeshLoop(Tris[i][0], Tris[i][1], Tris[i][2]));
	}
	Mesh.CalculateCenterAndBounds();

	Mesh.CalculateFaceLoops();

	CHECK_EQUALS("Coplanar-merge should collapse 12 triangles into 6 quad faces", Mesh.FaceLoops.Num(), 6);
	for (int32 i = 0; i < Mesh.FaceLoops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every recovered face should be a quad"), Mesh.FaceLoops[i].IsQuad());
	}

	Mesh.Validate();
	CHECK_MESSAGE(TEXT("Recovered FaceLoops should let CheckConvex accept the cube"), Mesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_CalculateFaceLoops_NoCoplanarGroups_KeepsTriangles,
	"NEXUS::UnitTests::NCore::FNRawMesh::CalculateFaceLoops::NoCoplanarGroups_KeepsTriangles",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A regular tetrahedron — no two faces share a plane, so recovery should keep every triangle as its own face.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{  10,  10,  10 },
		{  10, -10, -10 },
		{ -10,  10, -10 },
		{ -10, -10,  10 },
	};
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(0, 3, 1));
	Mesh.Loops.Add(FNRawMeshLoop(0, 2, 3));
	Mesh.Loops.Add(FNRawMeshLoop(1, 3, 2));
	Mesh.CalculateCenterAndBounds();

	Mesh.CalculateFaceLoops();

	CHECK_EQUALS("Tetrahedron has 4 non-coplanar faces — recovery should emit 4 triangle FaceLoops", Mesh.FaceLoops.Num(), 4);
	for (int32 i = 0; i < Mesh.FaceLoops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Each tetrahedron FaceLoop should remain a triangle"), Mesh.FaceLoops[i].IsTriangle());
	}
}

N_TEST_HIGH(FNRawMeshTests_CalculateFaceLoops_MixedMesh_RecoversWhereCoplanar,
	"NEXUS::UnitTests::NCore::FNRawMesh::CalculateFaceLoops::MixedMesh_RecoversWhereCoplanar",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cube (6 quads → 12 tris) plus one isolated, non-coplanar triangle. Recovery should collapse the cube into
	// 6 quads while leaving the lone triangle as its own face — verifying that disconnected components don't accidentally
	// merge with anything they share no edge with.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
		{ 100, 0, 0 }, { 110, 0, 0 }, { 100, 10, 0 },
	};
	static const int32 CubeTris[12][3] = {
		{ 0, 3, 2 }, { 0, 2, 1 }, { 4, 5, 6 }, { 4, 6, 7 },
		{ 0, 1, 5 }, { 0, 5, 4 }, { 1, 2, 6 }, { 1, 6, 5 },
		{ 2, 3, 7 }, { 2, 7, 6 }, { 3, 0, 4 }, { 3, 4, 7 },
	};
	for (int32 i = 0; i < 12; ++i)
	{
		Mesh.Loops.Add(FNRawMeshLoop(CubeTris[i][0], CubeTris[i][1], CubeTris[i][2]));
	}
	Mesh.Loops.Add(FNRawMeshLoop(8, 9, 10));
	Mesh.CalculateCenterAndBounds();

	Mesh.CalculateFaceLoops();

	CHECK_EQUALS("Cube collapses to 6 quads + 1 stray triangle = 7 faces total", Mesh.FaceLoops.Num(), 7);
	int32 QuadCount = 0;
	int32 TriCount = 0;
	for (const FNRawMeshLoop& Face : Mesh.FaceLoops)
	{
		if (Face.IsQuad()) QuadCount++;
		else if (Face.IsTriangle()) TriCount++;
	}
	CHECK_EQUALS("Recovered cube faces should be 6 quads", QuadCount, 6);
	CHECK_EQUALS("Stray non-coplanar triangle should remain a triangle", TriCount, 1);
}

N_TEST_HIGH(FNRawMeshTests_CalculateFaceLoops_NonTriangulatedInput_NoOp,
	"NEXUS::UnitTests::NCore::FNRawMesh::CalculateFaceLoops::NonTriangulatedInput_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Recovery only understands all-triangle input; a mesh that already has n-gons should be left alone so callers
	// that ran CalculateFaceLoops by mistake on a polygonal mesh don't corrupt their FaceLoops.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));

	Mesh.CalculateFaceLoops();

	CHECK_EQUALS("FaceLoops should stay empty when input contains a non-triangle loop", Mesh.FaceLoops.Num(), 0);
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_DifferentCenter, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::DifferentCenter", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));
	A.CalculateCenterAndBounds();
	A.Validate();

	FNRawMesh B = A;
	B.Center += FVector(1, 0, 0);
	CHECK_FALSE_MESSAGE(TEXT("Meshes with different Center should not compare equal"), A == B);
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_DifferentBounds, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::DifferentBounds", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));
	A.CalculateCenterAndBounds();
	A.Validate();

	FNRawMesh B = A;
	B.Bounds = FBox(FVector(-100, -100, -100), FVector(100, 100, 100));
	CHECK_FALSE_MESSAGE(TEXT("Meshes with different Bounds should not compare equal"), A == B);
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_SameLoopCount_DifferentIndices, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::SameLoopCount_DifferentIndices", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0), FVector(1, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));

	FNRawMesh B = A;
	B.Loops[0] = FNRawMeshLoop(0, 1, 3);
	CHECK_FALSE_MESSAGE(TEXT("Same loop count but different loop indices should not compare equal"), A == B);
}

N_TEST_HIGH(FNRawMeshTests_CalculateCenterAndBounds_AsymmetricMesh, "NEXUS::UnitTests::NCore::FNRawMesh::CalculateCenterAndBounds::AsymmetricMesh", N_TEST_CONTEXT_ANYWHERE)
{
	// Asymmetric vertex cloud — Center must be the arithmetic mean, Bounds must tightly enclose the cloud.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 20, 0), FVector(0, 0, 40) };

	Mesh.CalculateCenterAndBounds();

	CHECK_MESSAGE(TEXT("Center should be the arithmetic mean of the four vertices (5, 5, 10)"),
		Mesh.Center.Equals(FVector(5, 5, 10), 0.001));
	CHECK_MESSAGE(TEXT("Bounds min should be the per-axis minimum (0, 0, 0)"), Mesh.Bounds.Min.Equals(FVector(0, 0, 0)));
	CHECK_MESSAGE(TEXT("Bounds max should be the per-axis maximum (10, 20, 40)"), Mesh.Bounds.Max.Equals(FVector(10, 20, 40)));
	CHECK_MESSAGE(TEXT("HasBounds() should be true after CalculateCenterAndBounds"), Mesh.HasBounds());
}

N_TEST_HIGH(FNRawMeshTests_RotatedAroundPivot_NonOriginPivot, "NEXUS::UnitTests::NCore::FNRawMesh::RotatedAroundPivot::NonOriginPivot", N_TEST_CONTEXT_ANYWHERE)
{
	// Vertex sits at the pivot so it must stay put regardless of rotation, and Center must rotate alongside vertices.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(100, 0, 0), FVector(110, 0, 0) };
	Mesh.Center = FVector(105, 0, 0);

	Mesh.RotatedAroundPivot(FVector(100, 0, 0), FRotator(0.f, 90.f, 0.f));

	CHECK_MESSAGE(TEXT("Vertex at the pivot should remain at the pivot after rotation"),
		Mesh.Vertices[0].Equals(FVector(100, 0, 0), 0.01));
	CHECK_MESSAGE(TEXT("Center should rotate alongside the vertices around the pivot"),
		!Mesh.Center.Equals(FVector(105, 0, 0), 0.01));
}

N_TEST_CRITICAL(FNRawMeshTests_CreateDynamicMesh_TriangleMesh_CountsMatch, "NEXUS::UnitTests::NCore::FNRawMesh::CreateDynamicMesh::TriangleMesh_CountsMatch", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	const FDynamicMesh3 Dyn = Mesh.CreateDynamicMesh(false);

	CHECK_EQUALS("Resulting dynamic mesh should contain the source vertices", Dyn.VertexCount(), 3);
	CHECK_EQUALS("Resulting dynamic mesh should contain the source triangle", Dyn.TriangleCount(), 1);
}

N_TEST_HIGH(FNRawMeshTests_CreateDynamicMesh_NonTri_Error, "NEXUS::UnitTests::NCore::FNRawMesh::CreateDynamicMesh::NonTri_Error", N_TEST_CONTEXT_ANYWHERE)
{
	AddExpectedMessage(TEXT("must be triangulated"), ELogVerbosity::Error);

	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	const FDynamicMesh3 Dyn = Mesh.CreateDynamicMesh(false);

	CHECK_EQUALS("Non-triangulated input must produce an empty dynamic mesh", Dyn.TriangleCount(), 0);
}

N_TEST_HIGH(FNRawMeshTests_CreateDynamicMesh_ProcessMesh_EnablesVertexNormals, "NEXUS::UnitTests::NCore::FNRawMesh::CreateDynamicMesh::ProcessMesh_EnablesVertexNormals", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	const FDynamicMesh3 Dyn = Mesh.CreateDynamicMesh(true);

	CHECK_MESSAGE(TEXT("bProcessMesh=true should enable vertex normals on the dynamic mesh"), Dyn.HasVertexNormals());
	CHECK_EQUALS("Processed dynamic mesh should still contain the source triangle", Dyn.TriangleCount(), 1);
}

N_TEST_HIGH(FNRawMeshTests_ApplyScale_Identity_NoOp, "NEXUS::UnitTests::NCore::FNRawMesh::ApplyScale::Identity_NoOp", N_TEST_CONTEXT_ANYWHERE)
{
	// Identity scale hits the OneVector early-out — vertices, Center, and Bounds must all survive untouched
	// (a buggy implementation that always re-multiplied could introduce FP drift here).
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(1, 2, 3), FVector(4, 5, 6) };
	Mesh.CalculateCenterAndBounds();
	const FVector OriginalCenter = Mesh.Center;
	const FBox OriginalBounds = Mesh.Bounds;

	Mesh.ApplyScale(FVector::OneVector);

	CHECK_MESSAGE(TEXT("Identity scale must leave vertex 0 unchanged"), Mesh.Vertices[0].Equals(FVector(1, 2, 3)));
	CHECK_MESSAGE(TEXT("Identity scale must leave vertex 1 unchanged"), Mesh.Vertices[1].Equals(FVector(4, 5, 6)));
	CHECK_MESSAGE(TEXT("Identity scale must leave Center unchanged"), Mesh.Center.Equals(OriginalCenter));
	CHECK_MESSAGE(TEXT("Identity scale must leave Bounds unchanged"),
		Mesh.Bounds.Min.Equals(OriginalBounds.Min) && Mesh.Bounds.Max.Equals(OriginalBounds.Max));
}

N_TEST_HIGH(FNRawMeshTests_ApplyScale_Uniform_ScalesAllAxes, "NEXUS::UnitTests::NCore::FNRawMesh::ApplyScale::Uniform_ScalesAllAxes", N_TEST_CONTEXT_ANYWHERE)
{
	// Uniform 2x scale — every vertex doubles and the recompute path updates Center + Bounds.
	// HasBounds() below drives EnsureValidated, which calls CheckConvex; on a vertex-only mesh that
	// path warns by design (see CheckConvex_Empty_FalseWithWarning) — silence it here so the
	// validation-cache assertion can stand.
	AddExpectedMessage(TEXT("No vertices or loops were found in the FNRawMesh"), ELogVerbosity::Warning);

	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(1, 2, 3), FVector(4, 5, 6) };

	Mesh.ApplyScale(FVector(2.0, 2.0, 2.0));

	CHECK_MESSAGE(TEXT("Uniform 2x scale must double vertex 0"), Mesh.Vertices[0].Equals(FVector(2, 4, 6)));
	CHECK_MESSAGE(TEXT("Uniform 2x scale must double vertex 1"), Mesh.Vertices[1].Equals(FVector(8, 10, 12)));
	CHECK_MESSAGE(TEXT("Center must be recomputed from the scaled vertices"), Mesh.Center.Equals(FVector(5, 7, 9)));
	CHECK_MESSAGE(TEXT("Bounds.Min must reflect the scaled minimum"), Mesh.Bounds.Min.Equals(FVector(2, 4, 6)));
	CHECK_MESSAGE(TEXT("Bounds.Max must reflect the scaled maximum"), Mesh.Bounds.Max.Equals(FVector(8, 10, 12)));
	CHECK_MESSAGE(TEXT("HasBounds() must be true after the recompute"), Mesh.HasBounds());
}

N_TEST_HIGH(FNRawMeshTests_ApplyScale_NonUniform_PerAxisIndependent, "NEXUS::UnitTests::NCore::FNRawMesh::ApplyScale::NonUniform_PerAxisIndependent", N_TEST_CONTEXT_ANYWHERE)
{
	// Non-uniform scale (0.5, 2, 1) — proves per-axis independence; a regression that collapsed to a uniform
	// shortcut would visibly mangle this vertex.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(2, 4, 8) };

	Mesh.ApplyScale(FVector(0.5, 2.0, 1.0));

	CHECK_MESSAGE(TEXT("Non-uniform scale must apply independently per axis (0.5*2, 2*4, 1*8) = (1, 8, 8)"),
		Mesh.Vertices[0].Equals(FVector(1, 8, 8)));
}

N_TEST_HIGH(FNRawMeshTests_ApplyScale_Negative_MirrorsAndRecomputesBounds, "NEXUS::UnitTests::NCore::FNRawMesh::ApplyScale::Negative_MirrorsAndRecomputesBounds", N_TEST_CONTEXT_ANYWHERE)
{
	// Negative X scale mirrors the mesh across the YZ plane. The post-scale CalculateCenterAndBounds must
	// pick min/max from the mirrored positions — a naive "Bounds *= Scale" would leave Min > Max and break
	// downstream AABB tests.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(1, 0, 0), FVector(3, 0, 0) };

	Mesh.ApplyScale(FVector(-1.0, 1.0, 1.0));

	CHECK_MESSAGE(TEXT("Negative X scale must mirror vertex 0 to x=-1"), Mesh.Vertices[0].Equals(FVector(-1, 0, 0)));
	CHECK_MESSAGE(TEXT("Negative X scale must mirror vertex 1 to x=-3"), Mesh.Vertices[1].Equals(FVector(-3, 0, 0)));
	CHECK_MESSAGE(TEXT("Bounds.Min.X must be the post-mirror minimum (-3)"), FMath::IsNearlyEqual(Mesh.Bounds.Min.X, -3.0, 0.001));
	CHECK_MESSAGE(TEXT("Bounds.Max.X must be the post-mirror maximum (-1)"), FMath::IsNearlyEqual(Mesh.Bounds.Max.X, -1.0, 0.001));
}

N_TEST_HIGH(FNRawMeshTests_CalculateCenterAndBounds_Empty_HasBoundsFalse,
	"NEXUS::UnitTests::NCore::FNRawMesh::CalculateCenterAndBounds::Empty_HasBoundsFalse",
	N_TEST_CONTEXT_ANYWHERE)
{
	// CalculateCenterAndBounds must mirror Validate's CheckBounds contract: HasBounds() reports true only
	// when the underlying FBox is actually valid. Without this, callers that read HasBounds() to decide
	// whether to use the AABB get a false positive on empty meshes (e.g. DoesIntersect's bounds early-out
	// would treat the empty mesh's invalid FBox as a real AABB and produce surprising results).
	FNRawMesh Mesh;
	Mesh.CalculateCenterAndBounds();

	CHECK_MESSAGE(TEXT("Center must collapse to zero when no vertices were supplied"), Mesh.Center.IsZero());
	CHECK_FALSE_MESSAGE(TEXT("HasBounds() must report false when no vertices contributed an AABB"), Mesh.HasBounds());
	CHECK_FALSE_MESSAGE(TEXT("Underlying FBox must remain invalid"), Mesh.Bounds.IsValid != 0);
}

N_TEST_HIGH(FNRawMeshTests_Validate_SkippedWhenChaosGenerated,
	"NEXUS::UnitTests::NCore::FNRawMesh::Validate::SkippedWhenChaosGenerated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Validate's early-out at bIsChaosGenerated==true: subsequent calls must leave cached flags alone.
	// Build a Chaos-generated hull via ToConvexHull, then deliberately break convexity at the vertex level.
	// A correct Validate must SKIP the recompute and IsConvex() must still report true.
	FNRawMesh Cloud;
	Cloud.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
	};
	// Deliberately skip Validate on Cloud so bIsConvex stays false and ToConvexHull doesn't early-return.

	FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Cloud);
	CHECK_MESSAGE(TEXT("Hull of a cube cloud must initially report convex"), Hull.IsConvex());

	// Yank a single hull vertex far outside — geometrically non-convex now.
	Hull.Vertices[0] = FVector(0, 0, 10000);
	Hull.Validate();

	CHECK_MESSAGE(TEXT("Validate must skip when bIsChaosGenerated==true, so IsConvex remains true despite the broken vertex"),
		Hull.IsConvex());
}

#endif //WITH_TESTS
