// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge
{
	// Walk every loop looking for the directed pair (A → B) wrap-aware. Used to assert that the
	// pre-split edge no longer exists on any face, i.e. the split was T-junction free.
	static bool AnyLoopHasEdge(const FNRawMesh& Mesh, const int32 A, const int32 B)
	{
		for (const FNRawMeshLoop& Loop : Mesh.Loops)
		{
			const int32 Count = Loop.Indices.Num();
			for (int32 i = 0; i < Count; i++)
			{
				const int32 Current = Loop.Indices[i];
				const int32 Next = Loop.Indices[(i + 1) % Count];
				if ((Current == A && Next == B) || (Current == B && Next == A))
				{
					return true;
				}
			}
		}
		return false;
	}

	static bool LoopContainsIndex(const FNRawMeshLoop& Loop, const int32 Index)
	{
		return Loop.Indices.Contains(Index);
	}

	static double TriangleArea2D(const FVector& A, const FVector& B, const FVector& C)
	{
		const double Cross = (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
		return FMath::Abs(Cross) * 0.5;
	}
}

N_TEST_CRITICAL(FNRawMeshTests_SplitEdge_SingleTriangle_ReturnsNewIndex,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::SingleTriangle_ReturnsNewIndex",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Sanity: the inserted vertex is appended at the end of Vertices, and the returned index
	// equals that new slot. Callers (notably the visualizer) rely on this so they can immediately
	// select the freshly created vertex for a follow-up drag.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.CalculateCenterAndBounds();
	Mesh.Validate();

	const int32 NewIndex = Mesh.SplitEdge(0, 1);

	CHECK_EQUALS("SplitEdge should return the appended vertex's index", NewIndex, 3);
	CHECK_EQUALS("Vertex buffer should grow by exactly one entry", Mesh.Vertices.Num(), 4);
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_MidpointPositionCorrect,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::MidpointPositionCorrect",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The inserted vertex must sit exactly at (V[A] + V[B]) * 0.5 — any drift would change the
	// hull shape, which is the opposite of what an edge split is supposed to do.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	const int32 NewIndex = Mesh.SplitEdge(0, 1);

	CHECK_MESSAGE(TEXT("Inserted vertex must be the exact midpoint of the source edge"),
		Mesh.Vertices[NewIndex].Equals(FVector(5, 0, 0)));
}

N_TEST_CRITICAL(FNRawMeshTests_SplitEdge_SingleTriangle_LoopsStayTriangulated,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::SingleTriangle_LoopsStayTriangulated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A triangle becomes a quad after insertion but must be immediately fan-triangulated from the
	// new midpoint vertex — Loops is contractually triangulated and CreateDynamicMesh / CalculateFaceLoops
	// / CheckConvex's triangle-path all rely on that invariant.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	const int32 NewIndex = Mesh.SplitEdge(0, 1);

	CHECK_EQUALS("Triangle split should produce exactly two output triangles", Mesh.Loops.Num(), 2);
	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every emitted loop must remain a triangle"), Mesh.Loops[i].IsTriangle());
		CHECK_MESSAGE(TEXT("Every emitted triangle must reference the new midpoint vertex (fan-from-M invariant)"),
			NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::LoopContainsIndex(Mesh.Loops[i], NewIndex));
	}
}

N_TEST_CRITICAL(FNRawMeshTests_SplitEdge_SingleTriangle_AreaPreserved,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::SingleTriangle_AreaPreserved",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Splitting bisects a triangle along the M↔opposite-corner line. The two output triangles must
	// be non-overlapping and together cover the source — any drift indicates a winding bug or a
	// degenerate emitted triangle.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	const double SourceArea = NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::TriangleArea2D(
		Mesh.Vertices[0], Mesh.Vertices[1], Mesh.Vertices[2]);

	Mesh.SplitEdge(0, 1);

	double TotalArea = 0.0;
	for (const FNRawMeshLoop& Loop : Mesh.Loops)
	{
		TotalArea += NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::TriangleArea2D(
			Mesh.Vertices[Loop.Indices[0]],
			Mesh.Vertices[Loop.Indices[1]],
			Mesh.Vertices[Loop.Indices[2]]);
	}

	CHECK_MESSAGE(TEXT("Sum of post-split triangle areas must equal the source triangle area — any overlap or gap indicates a broken split"),
		FMath::IsNearlyEqual(TotalArea, SourceArea, 0.001));
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_DirectionAgnostic,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::DirectionAgnostic",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The same undirected edge appears as A→B in one face and B→A in the adjacent face. Callers
	// shouldn't have to know which direction the face winds — both orderings must produce a
	// successful split at the same midpoint.
	FNRawMesh A;
	A.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	A.Loops.Add(FNRawMeshLoop(0, 1, 2));

	FNRawMesh B = A;

	const int32 AIdx = A.SplitEdge(0, 1);
	const int32 BIdx = B.SplitEdge(1, 0);

	CHECK_MESSAGE(TEXT("Forward and reverse calls must both succeed"), AIdx != INDEX_NONE && BIdx != INDEX_NONE);
	CHECK_MESSAGE(TEXT("Forward and reverse calls must produce the same midpoint position"),
		A.Vertices[AIdx].Equals(B.Vertices[BIdx]));
}

N_TEST_CRITICAL(FNRawMeshTests_SplitEdge_SharedEdge_NoTJunction,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::SharedEdge_NoTJunction",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two triangles meeting along an internal edge must both be updated, otherwise the result has
	// a T-junction (one face uses the midpoint, the other still uses the original edge), which
	// breaks convexity testing and produces cracks in rendering / physics downstream.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(0, 2, 3));

	const int32 NewIndex = Mesh.SplitEdge(0, 2);

	CHECK_MESSAGE(TEXT("Splitting an interior shared edge must return a valid new vertex"), NewIndex != INDEX_NONE);
	CHECK_EQUALS("Two source triangles around a shared edge should split into four output triangles", Mesh.Loops.Num(), 4);
	CHECK_FALSE_MESSAGE(TEXT("No loop must still contain the pre-split (0 → 2) directed edge"),
		NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::AnyLoopHasEdge(Mesh, 0, 2));
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_SharedEdge_AllTrianglesAdoptMidpoint,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::SharedEdge_AllTrianglesAdoptMidpoint",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every triangle that used to own the shared edge must reference M afterwards. This is the
	// positive-side counterpart of the T-junction check: not just "the old edge is gone" but
	// "every face that used to have it now has M instead".
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(0, 2, 3));

	const int32 NewIndex = Mesh.SplitEdge(0, 2);

	for (int32 i = 0; i < Mesh.Loops.Num(); i++)
	{
		CHECK_MESSAGE(TEXT("Every output triangle must reference the new midpoint vertex"),
			NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::LoopContainsIndex(Mesh.Loops[i], NewIndex));
	}
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_InvalidIndex_NoOp,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::InvalidIndex_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Out-of-range endpoint must bail before mutating anything — no appended vertex, no rewritten
	// loops. Otherwise random caller bugs leave the mesh in a corrupt state.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	const int32 Result = Mesh.SplitEdge(0, 99);

	CHECK_EQUALS("Out-of-range index should return INDEX_NONE", Result, INDEX_NONE);
	CHECK_EQUALS("Vertex buffer must not have grown on failure", Mesh.Vertices.Num(), 3);
	CHECK_EQUALS("Loop list must not have been rewritten on failure", Mesh.Loops.Num(), 1);
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_EqualIndices_NoOp,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::EqualIndices_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// "Split the edge from V to V" is meaningless — must reject before any mutation.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	const int32 Result = Mesh.SplitEdge(1, 1);

	CHECK_EQUALS("Equal endpoint indices should return INDEX_NONE", Result, INDEX_NONE);
	CHECK_EQUALS("Vertex buffer must not have grown on failure", Mesh.Vertices.Num(), 3);
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_EdgeNotInAnyLoop_NoOp,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::EdgeNotInAnyLoop_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Both endpoints are valid vertex indices, but no Loop or FaceLoop has them as a consecutive
	// pair. Appending the midpoint anyway would orphan it (no face references it), which would
	// skew CalculateCenterAndBounds and confuse downstream geometry consumers.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0), // triangle
		FVector(100, 100, 100)                                  // unreferenced
	};
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	const int32 Result = Mesh.SplitEdge(0, 3);

	CHECK_EQUALS("Unreferenced edge must return INDEX_NONE", Result, INDEX_NONE);
	CHECK_EQUALS("No orphan midpoint vertex should have been appended", Mesh.Vertices.Num(), 4);
	CHECK_EQUALS("Source loop must remain unchanged", Mesh.Loops.Num(), 1);
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_CenterAndBoundsRefreshed,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::CenterAndBoundsRefreshed",
	N_TEST_CONTEXT_ANYWHERE)
{
	// SplitEdge must call CalculateCenterAndBounds() so cached Center / Bounds / HasBounds reflect
	// the new vertex set. The cell visualizer reads these to position its gizmos.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	Mesh.SplitEdge(0, 1);

	const FVector ExpectedCenter = (FVector(0, 0, 0) + FVector(10, 0, 0) + FVector(0, 10, 0) + FVector(5, 0, 0)) / 4.0;
	CHECK_MESSAGE(TEXT("Center must be the arithmetic mean of the post-split vertex set"),
		Mesh.Center.Equals(ExpectedCenter, 0.001));
	CHECK_MESSAGE(TEXT("HasBounds() must report true after the post-split recompute"), Mesh.HasBounds());
}

N_TEST_CRITICAL(FNRawMeshTests_SplitEdge_FaceLoops_OuterEdge_BothFacesGetMidpoint,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::FaceLoops_OuterEdge_BothFacesGetMidpoint",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When FaceLoops describes the polygonal source faces, splitting an outer edge must splice M
	// into both adjacent face loops — that's what keeps CheckConvex on the FaceLoops path (avoiding
	// the per-triangle plane-drift trap) and what keeps the polygonal description authoritative.
	FNRawMesh Mesh;
	Mesh.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
	};
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 3, 2, 1)); // -Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(4, 5, 6, 7)); // +Z
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 1, 5, 4)); // -Y (owns edge 0→1)
	Mesh.FaceLoops.Add(FNRawMeshLoop(1, 2, 6, 5)); // +X
	Mesh.FaceLoops.Add(FNRawMeshLoop(2, 3, 7, 6)); // +Y
	Mesh.FaceLoops.Add(FNRawMeshLoop(3, 0, 4, 7)); // -X
	Mesh.Loops = Mesh.FaceLoops;
	Mesh.ConvertToTriangles();
	Mesh.CalculateCenterAndBounds();

	const int32 NewIndex = Mesh.SplitEdge(0, 1);

	CHECK_MESSAGE(TEXT("Cube outer-edge split must return a valid new vertex"), NewIndex != INDEX_NONE);

	// Edge 0↔1 is shared by -Z (winding 0,3,2,1) and -Y (winding 0,1,5,4). Both should now host M.
	int32 FacesContainingMidpoint = 0;
	for (const FNRawMeshLoop& Face : Mesh.FaceLoops)
	{
		if (NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::LoopContainsIndex(Face, NewIndex))
		{
			FacesContainingMidpoint++;
		}
	}
	CHECK_EQUALS("Exactly the two FaceLoops sharing the split edge should now contain the midpoint",
		FacesContainingMidpoint, 2);

	// FaceLoops's polygonal description must still describe convex polygons; CheckConvex walks them.
	CHECK_MESSAGE(TEXT("Cube must still report convex after an outer-edge split (FaceLoops path)"), Mesh.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_FaceLoops_DiagonalEdge_OnlyLoopsTouched,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::FaceLoops_DiagonalEdge_OnlyLoopsTouched",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A fan-triangulation diagonal lives in Loops but not in FaceLoops. The function should still
	// split it (because Loops references the edge) but FaceLoops must remain untouched — the
	// polygonal source still describes the same n-gonal faces.
	//
	// Manual fan-triangulation from vertex 0 so the diagonal index is deterministic: ear-clipping
	// inside ConvertToTriangles picks ears based on signed-area orientation and would give the
	// other diagonal, which is a different correct triangulation but not the one this test asserts.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(10, 10, 0), FVector(0, 10, 0) };
	Mesh.FaceLoops.Add(FNRawMeshLoop(0, 1, 2, 3));
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));
	Mesh.Loops.Add(FNRawMeshLoop(0, 2, 3));
	Mesh.CalculateCenterAndBounds();

	// Edge 0↔2 is the fan diagonal — owned by both Loops triangles, absent from the FaceLoop.
	const int32 NewIndex = Mesh.SplitEdge(0, 2);
	CHECK_MESSAGE(TEXT("Diagonal split (edge owned by Loops only) must still succeed"), NewIndex != INDEX_NONE);

	for (const FNRawMeshLoop& Face : Mesh.FaceLoops)
	{
		CHECK_FALSE_MESSAGE(TEXT("FaceLoops must not have picked up the midpoint for a diagonal-only edge"),
			NEXUS::UnitTests::NCore::FNRawMeshHarness::SplitEdge::LoopContainsIndex(Face, NewIndex));
	}
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_ChaosGeneratedCleared,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::ChaosGeneratedCleared",
	N_TEST_CONTEXT_ANYWHERE)
{
	// SplitEdge is a topology change — the chaos provenance no longer applies, and Validate must
	// resume recomputing flags. Verify by breaking convexity post-split: if bIsChaosGenerated were
	// still set, Validate would skip and IsConvex would stay cached at true.
	FNRawMesh Cloud;
	Cloud.Vertices = {
		{ -5, -5, -5 }, { +5, -5, -5 }, { +5, +5, -5 }, { -5, +5, -5 },
		{ -5, -5, +5 }, { +5, -5, +5 }, { +5, +5, +5 }, { -5, +5, +5 },
	};
	FNRawMesh Hull = FNRawMeshUtils::ToConvexHull(Cloud);
	CHECK_MESSAGE(TEXT("Chaos-generated cube hull must report convex out of the gate"), Hull.IsConvex());

	const int32 V0 = Hull.Loops[0].Indices[0];
	const int32 V1 = Hull.Loops[0].Indices[1];
	const int32 NewIndex = Hull.SplitEdge(V0, V1);
	CHECK_MESSAGE(TEXT("SplitEdge against a chaos-generated hull must still succeed"), NewIndex != INDEX_NONE);

	// Yank an original hull vertex far outside — geometrically non-convex now.
	Hull.Vertices[V0] = FVector(0, 0, 10000);
	Hull.Validate();

	CHECK_FALSE_MESSAGE(TEXT("Validate must recompute after SplitEdge clears bIsChaosGenerated, so the broken vertex flips IsConvex to false"),
		Hull.IsConvex());
}

N_TEST_HIGH(FNRawMeshTests_SplitEdge_HasNonTrisFlagRefreshed,
	"NEXUS::UnitTests::NCore::FNRawMesh::SplitEdge::HasNonTrisFlagRefreshed",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Triangulated input + edge split → still all triangles. HasNonTris must reflect that
	// post-Validate, otherwise downstream code (CreateDynamicMesh) may bail thinking the mesh
	// still needs re-triangulation.
	FNRawMesh Mesh;
	Mesh.Vertices = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	Mesh.Loops.Add(FNRawMeshLoop(0, 1, 2));

	Mesh.SplitEdge(0, 1);

	CHECK_FALSE_MESSAGE(TEXT("HasNonTris must be false after splitting an edge of an all-triangle mesh"), Mesh.HasNonTris());
}

#endif //WITH_TESTS
