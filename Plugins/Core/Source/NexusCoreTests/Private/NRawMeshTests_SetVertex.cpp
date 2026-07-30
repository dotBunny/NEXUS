// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

/**
 * Coverage for FNRawMesh::SetVertex and the cache staleness it exists to prevent.
 *
 * These mirror what the cell-hull vertex gizmo does: move a vertex, evaluate the result, and sometimes put it back.
 * Doing that by assigning into the public Vertices array leaves the validation flags and the face-plane cache
 * describing the geometry as it was, which is a wrong answer rather than a slow one.
 */
namespace NEXUS::UnitTests::NCore::FNRawMeshSetVertexHarness
{
	constexpr double BoxHalfExtent = 100.0;

	/** Depths here are in the tens of units; this absorbs float round-trip drift between two build paths. */
	constexpr float Tolerance = 0.01f;

	static FNRawMesh MakeBoxHull()
	{
		return FNRawMeshUtils::MakeBoxHull(FBox(FVector(-BoxHalfExtent), FVector(BoxHalfExtent)));
	}

	/** @return The index of the vertex closest to Target, or INDEX_NONE. */
	static int32 FindVertex(const FNRawMesh& Mesh, const FVector& Target)
	{
		int32 Best = INDEX_NONE;
		double BestDistSq = MAX_dbl;
		for (int32 i = 0; i < Mesh.Vertices.Num(); ++i)
		{
			const double DistSq = FVector::DistSquared(Mesh.Vertices[i], Target);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = i;
			}
		}
		return Best;
	}

	/** A position that pulls a box corner inward far enough to make the hull concave. */
	static FVector ConcavePosition()
	{
		return FVector(BoxHalfExtent * 0.1, BoxHalfExtent * 0.1, BoxHalfExtent * 0.1);
	}
}

N_TEST_HIGH(FNRawMeshTests_SetVertex_RestoresConvexityVerdict,
	"NEXUS::UnitTests::NCore::FNRawMesh::SetVertex::RestoresConvexityVerdict",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The cell-hull gizmo's reject path: move a vertex, discover the hull went concave, put the vertex back. Once
	// restored, the mesh must report the convexity of the geometry it actually holds — not the verdict computed for
	// the position that was thrown away. Assigning into Vertices directly leaves that verdict latched.
	using namespace NEXUS::UnitTests::NCore::FNRawMeshSetVertexHarness;

	FNRawMesh Hull = MakeBoxHull();
	const int32 Corner = FindVertex(Hull, FVector(BoxHalfExtent));
	CHECK_MESSAGE(TEXT("The box hull should expose a corner vertex to move."), Corner != INDEX_NONE)
	CHECK_MESSAGE(TEXT("A box hull starts convex."), Hull.IsConvex())

	const FVector PreviousPosition = Hull.Vertices[Corner];

	// Pull the corner inward, then evaluate — exactly what the gizmo does before deciding whether to keep the move.
	Hull.SetVertex(Corner, ConcavePosition());
	Hull.CalculateCenterAndBounds();
	CHECK_FALSE_MESSAGE(TEXT("Denting a corner inward must make the hull concave, or this proves nothing."), Hull.IsConvex())

	// Rejected — put it back.
	Hull.SetVertex(Corner, PreviousPosition);
	Hull.CalculateCenterAndBounds();

	CHECK_MESSAGE(TEXT("A restored vertex must restore the convexity verdict."), Hull.IsConvex())
}

N_TEST_HIGH(FNRawMeshTests_SetVertex_DepthMatchesFreshlyBuiltMesh,
	"NEXUS::UnitTests::NCore::FNRawMesh::SetVertex::DepthMatchesFreshlyBuiltMesh",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The convex depth test measures against cached face planes. Querying once builds that cache; editing a vertex
	// afterwards must discard it, or every later measurement is taken against surfaces that have since moved. The
	// oracle is a mesh built directly at the edited geometry — the edited mesh has to agree with it.
	using namespace NEXUS::UnitTests::NCore::FNRawMeshSetVertexHarness;

	// Shrink the box along one axis, so the nearest surface genuinely moves and a stale plane gives a stale depth.
	const FBox EditedBox(FVector(-BoxHalfExtent), FVector(BoxHalfExtent * 0.25, BoxHalfExtent, BoxHalfExtent));
	const FVector SamplePoint(0.0, 0.0, 0.0);

	FNRawMesh Edited = MakeBoxHull();

	// Query first, so the face-plane cache is populated against the original geometry.
	const float DepthBeforeEdit = FNRawMeshUtils::GetIntersectDepth(Edited, SamplePoint);
	CHECK_MESSAGE(TEXT("The sample point starts inside the hull."), DepthBeforeEdit > 0.f)
	CHECK_MESSAGE(TEXT("Querying the convex path should populate the face-plane cache."), Edited.HasCachedFacePlanes())

	// Now move every vertex sitting on the +X face inward, reshaping the hull under the populated cache.
	for (int32 i = 0; i < Edited.Vertices.Num(); ++i)
	{
		if (FMath::IsNearlyEqual(Edited.Vertices[i].X, BoxHalfExtent))
		{
			FVector Moved = Edited.Vertices[i];
			Moved.X = BoxHalfExtent * 0.25;
			Edited.SetVertex(i, Moved);
		}
	}
	Edited.CalculateCenterAndBounds();

	const FNRawMesh Fresh = FNRawMeshUtils::MakeBoxHull(EditedBox);
	const float FreshDepth = FNRawMeshUtils::GetIntersectDepth(Fresh, SamplePoint);
	const float EditedDepth = FNRawMeshUtils::GetIntersectDepth(Edited, SamplePoint);

	CHECK_MESSAGE(TEXT("The edit must actually change the measured depth, or this proves nothing."),
		!FMath::IsNearlyEqual(FreshDepth, DepthBeforeEdit, Tolerance))
	CHECK_MESSAGE(TEXT("An edited mesh must measure as its geometry now is, not as it was when the cache was built."),
		FMath::IsNearlyEqual(EditedDepth, FreshDepth, Tolerance))
}

N_TEST_MEDIUM(FNRawMeshTests_SetVertex_IgnoresOutOfRangeIndex,
	"NEXUS::UnitTests::NCore::FNRawMesh::SetVertex::IgnoresOutOfRangeIndex",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The gizmo can capture a vertex index that a concurrent hull recompute then shrinks past, so an out-of-range
	// index must be a no-op rather than a write past the end.
	using namespace NEXUS::UnitTests::NCore::FNRawMeshSetVertexHarness;

	FNRawMesh Hull = MakeBoxHull();
	const int32 VertexCount = Hull.Vertices.Num();

	Hull.SetVertex(VertexCount, FVector(1000.0));
	Hull.SetVertex(-1, FVector(1000.0));

	CHECK_EQUALS("An out-of-range write must not change the vertex count.", Hull.Vertices.Num(), VertexCount)
	CHECK_MESSAGE(TEXT("An out-of-range write must leave the hull untouched and convex."), Hull.IsConvex())
}

#endif //WITH_TESTS
