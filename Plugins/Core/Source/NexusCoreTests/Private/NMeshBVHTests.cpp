// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NMeshBVH.h"
#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNMeshBVHHarness
{
	/** Depths and distances are in the ~50-unit range; this absorbs float round-trip drift between the two paths. */
	constexpr float Tolerance = 0.02f;

	/**
	 * Builds a non-convex mesh: two disjoint axis-aligned boxes merged into one origin-anchored mesh, exactly the
	 * shape World Assembly's collision merge produces (triangulated Loops, non-convex, no non-tri loops).
	 */
	static FNRawMesh MakeTwoBoxMesh()
	{
		FNRawMesh Mesh = FNRawMeshUtils::MakeBoxHull(FBox(FVector(-50.0), FVector(50.0)));
		FNRawMeshUtils::CombineMesh(FTransform::Identity, Mesh, FTransform::Identity,
			FNRawMeshUtils::MakeBoxHull(FBox(FVector(150.0, -50.0, -50.0), FVector(250.0, 50.0, 50.0))));
		return Mesh;
	}
}

N_TEST_HIGH(FNMeshBVHTests_GetPointDepth_MatchesBruteForce,
	"NEXUS::UnitTests::NCore::FNMeshBVH::GetPointDepth::MatchesBruteForce",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the BVH reproduces FNRawMeshUtils::GetIntersectDepth's per-point penetration on a non-convex mesh —
	// the whole point of the accelerator is to be a drop-in for that brute-force sweep.
	using namespace NEXUS::UnitTests::NCore::FNMeshBVHHarness;

	const FNRawMesh Mesh = MakeTwoBoxMesh();
	CHECK_FALSE_MESSAGE(TEXT("The merged two-box mesh must be non-convex to exercise the accelerated path."), Mesh.IsConvex())

	const FNMeshBVH BVH(Mesh);
	CHECK_FALSE_MESSAGE(TEXT("A mesh with triangles should produce a non-empty BVH."), BVH.IsEmpty())

	const TArray<FVector> SamplePoints = {
		FVector(0, 0, 0),        // deep inside box A
		FVector(40, 0, 0),       // just inside box A near its +X face
		FVector(0, 0, 45),       // just inside box A near its +Z face
		FVector(200, 0, 0),      // deep inside box B
		FVector(160, 20, -30),   // off-center inside box B
		FVector(100, 0, 0),      // in the gap between the boxes (outside)
		FVector(0, 0, 500),      // far outside every box (AABB reject)
		FVector(-500, 0, 0),     // far outside on the other side
	};

	for (const FVector& Point : SamplePoints)
	{
		const float Reference = FNRawMeshUtils::GetIntersectDepth(Mesh, FVector::ZeroVector, FRotator::ZeroRotator, Point);
		const float Accelerated = BVH.GetPointDepth(Point);
		if (!FMath::IsNearlyEqual(Accelerated, Reference, Tolerance))
		{
			ADD_ERROR(FString::Printf(TEXT("BVH depth %.4f disagreed with brute-force %.4f at (%.0f, %.0f, %.0f)."),
				Accelerated, Reference, Point.X, Point.Y, Point.Z));
		}
	}
}

N_TEST_HIGH(FNMeshBVHTests_GetPointDepth_KnownValues,
	"NEXUS::UnitTests::NCore::FNMeshBVH::GetPointDepth::KnownValues",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies absolute depths (not just agreement with the brute-force path, which shares the same distance kernel).
	using namespace NEXUS::UnitTests::NCore::FNMeshBVHHarness;

	const FNMeshBVH BVH(MakeTwoBoxMesh());

	// Box A spans [-50, 50]; its center is 50 from every face.
	CHECK_MESSAGE(TEXT("Box A center depth should be ~50."), FMath::IsNearlyEqual(BVH.GetPointDepth(FVector(0, 0, 0)), 50.f, Tolerance))
	// 10 units inside the +X face of box A.
	CHECK_MESSAGE(TEXT("A point 10 inside a face should report depth ~10."), FMath::IsNearlyEqual(BVH.GetPointDepth(FVector(40, 0, 0)), 10.f, Tolerance))
	// The gap between the boxes is empty space.
	CHECK_MESSAGE(TEXT("A point in the gap should report no penetration (-1)."), BVH.GetPointDepth(FVector(100, 0, 0)) < 0.f)
	// Well outside the merged AABB.
	CHECK_MESSAGE(TEXT("A far point should report no penetration (-1)."), BVH.GetPointDepth(FVector(0, 0, 500)) < 0.f)
}

N_TEST_MEDIUM(FNMeshBVHTests_IsPointInside_MatchesBruteForce,
	"NEXUS::UnitTests::NCore::FNMeshBVH::IsPointInside::MatchesBruteForce",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the accelerated parity ray cast agrees with FNRawMeshUtils::IsRelativePointInside point-for-point.
	using namespace NEXUS::UnitTests::NCore::FNMeshBVHHarness;

	const FNRawMesh Mesh = MakeTwoBoxMesh();
	const FNMeshBVH BVH(Mesh);

	const TArray<FVector> SamplePoints = {
		FVector(0, 0, 0), FVector(200, 0, 0), FVector(30, -20, 10),
		FVector(100, 0, 0), FVector(0, 0, 500), FVector(300, 300, 300),
	};

	for (const FVector& Point : SamplePoints)
	{
		// The mesh is origin-anchored (baked), so a world-space point is already in its local frame.
		const bool Reference = FNRawMeshUtils::IsRelativePointInside(Mesh, Point);
		const bool Accelerated = BVH.IsPointInside(Point);
		if (Reference != Accelerated)
		{
			ADD_ERROR(FString::Printf(TEXT("BVH containment %d disagreed with brute-force %d at (%.0f, %.0f, %.0f)."),
				Accelerated, Reference, Point.X, Point.Y, Point.Z));
		}
	}
}

N_TEST_MEDIUM(FNMeshBVHTests_Empty_ReturnsEmptyAnswers,
	"NEXUS::UnitTests::NCore::FNMeshBVH::Empty::ReturnsEmptyAnswers",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies a BVH over a mesh with no triangles is handled cleanly.
	const FNMeshBVH BVH{FNRawMesh()};
	CHECK_MESSAGE(TEXT("An empty mesh yields an empty BVH."), BVH.IsEmpty())
	CHECK_MESSAGE(TEXT("An empty BVH reports no penetration for any point."), BVH.GetPointDepth(FVector::ZeroVector) < 0.f)
	CHECK_FALSE_MESSAGE(TEXT("An empty BVH contains no points."), BVH.IsPointInside(FVector::ZeroVector))
}

#endif //WITH_TESTS
