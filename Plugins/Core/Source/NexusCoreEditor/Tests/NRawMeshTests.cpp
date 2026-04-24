// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMesh.h"
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
	CHECK_MESSAGE(TEXT("Identical meshes should compare equal"), A.IsEqual(B));
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
	CHECK_FALSE_MESSAGE(TEXT("Meshes with different vertex positions should not compare equal"), A.IsEqual(B));
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_DifferentLoopCount, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::DifferentLoopCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0), FVector(1, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));

	FNRawMesh B = A;
	B.Loops.Add(FNRawMeshLoop(1, 3, 2));
	CHECK_FALSE_MESSAGE(TEXT("Meshes with differing loop counts should not compare equal"), A.IsEqual(B));
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
	// emit a "falling back to fan" warning and still reduce the loop to n-2 triangle entries.
	AddExpectedMessage(TEXT("Ear-clipping failed on loop"), ELogVerbosity::Warning);

	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(2, 0, 0), FVector(3, 0, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2, 3));
	// Deliberately skip Validate() so bIsConvex stays false and the ear-clipping branch runs on this degenerate input.

	Mesh.ConvertToTriangles();

	CHECK_EQUALS("Degenerate collinear quad should still yield n-2 = 2 triangle entries via the fan fallback", Mesh.Loops.Num(), 2);
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

N_TEST_HIGH(FNRawMeshTests_IsEqual_DifferentCenter, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::DifferentCenter", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));
	A.CalculateCenterAndBounds();
	A.Validate();

	FNRawMesh B = A;
	B.Center += FVector(1, 0, 0);
	CHECK_FALSE_MESSAGE(TEXT("Meshes with different Center should not compare equal"), A.IsEqual(B));
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
	CHECK_FALSE_MESSAGE(TEXT("Meshes with different Bounds should not compare equal"), A.IsEqual(B));
}

N_TEST_HIGH(FNRawMeshTests_IsEqual_SameLoopCount_DifferentIndices, "NEXUS::UnitTests::NCore::FNRawMesh::IsEqual::SameLoopCount_DifferentIndices", N_TEST_CONTEXT_ANYWHERE)
{
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0), FVector(1, 1, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));

	FNRawMesh B = A;
	B.Loops[0] = FNRawMeshLoop(0, 1, 3);
	CHECK_FALSE_MESSAGE(TEXT("Same loop count but different loop indices should not compare equal"), A.IsEqual(B));
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

#endif //WITH_TESTS
