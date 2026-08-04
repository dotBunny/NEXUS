// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::UnitTests::NCore::FNMakeConvexPrismHarness
{
	/** A square cap of half-extent Extent, in the XZ plane at the given Y, wound counter-clockwise seen from -Y. */
	static TStaticArray<FVector, 4> MakeCap(const double Y, const double Extent)
	{
		TStaticArray<FVector, 4> Corners;
		Corners[0] = FVector(-Extent, Y, Extent);
		Corners[1] = FVector(-Extent, Y, -Extent);
		Corners[2] = FVector(Extent, Y, -Extent);
		Corners[3] = FVector(Extent, Y, Extent);
		return Corners;
	}
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_MakeConvexPrism_Topology_EightVertsSixQuadsTwelveTris,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::MakeConvexPrism::Topology_EightVertsSixQuadsTwelveTris",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A prism carries the same production-shaped topology as a box hull: 8 corners, 6 quad FaceLoops, 12 triangles.
	using namespace NEXUS::UnitTests::NCore::FNMakeConvexPrismHarness;

	const FNRawMesh Prism = FNRawMeshUtils::MakeConvexPrism(MakeCap(0.0, 50.0), MakeCap(100.0, 50.0));

	CHECK_EQUALS("A prism must have 8 vertices.", Prism.Vertices.Num(), 8)
	CHECK_EQUALS("A prism must have 6 polygonal faces.", Prism.FaceLoops.Num(), 6)
	CHECK_EQUALS("A prism must have 12 triangle loops.", Prism.Loops.Num(), 12)

	for (const FNRawMeshLoop& Face : Prism.FaceLoops)
	{
		CHECK_MESSAGE(TEXT("Every FaceLoop on a prism must be a quad."), Face.IsQuad())
	}
	for (const FNRawMeshLoop& Loop : Prism.Loops)
	{
		CHECK_MESSAGE(TEXT("Every Loop on a prism must be a triangle."), Loop.IsTriangle())
	}
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_MakeConvexPrism_IsConvex_RegardlessOfCapWinding,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::MakeConvexPrism::IsConvex_RegardlessOfCapWinding",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Callers order socket corners to suit their own geometry, so the prism detects a cap wound against the sweep
	// and flips rather than emitting inward-facing geometry. A box-shaped prism must read convex either way.
	using namespace NEXUS::UnitTests::NCore::FNMakeConvexPrismHarness;

	const TStaticArray<FVector, 4> Near = MakeCap(0.0, 50.0);
	const TStaticArray<FVector, 4> Far = MakeCap(100.0, 50.0);

	CHECK_MESSAGE(TEXT("A box-shaped prism must report convex."),
		FNRawMeshUtils::MakeConvexPrism(Near, Far).IsConvex())

	// Same solid, caps supplied the other way round.
	CHECK_MESSAGE(TEXT("A prism must report convex with its caps supplied in the opposite order."),
		FNRawMeshUtils::MakeConvexPrism(Far, Near).IsConvex())

	// Same solid again, with the corner order reversed so the near cap's winding opposes the sweep.
	TStaticArray<FVector, 4> ReversedNear;
	TStaticArray<FVector, 4> ReversedFar;
	for (int32 i = 0; i < 4; i++)
	{
		ReversedNear[i] = Near[3 - i];
		ReversedFar[i] = Far[3 - i];
	}
	CHECK_MESSAGE(TEXT("A prism must report convex when its caps are wound against the sweep direction."),
		FNRawMeshUtils::MakeConvexPrism(ReversedNear, ReversedFar).IsConvex())
}

N_TEST_HIGH(FNRawMeshUtilsTests_MakeConvexPrism_BoundsSpanBothCaps,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::MakeConvexPrism::BoundsSpanBothCaps",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Bounds drive the broadphase rejection in front of every intersection test, so they have to enclose the whole
	// swept solid rather than either cap alone.
	using namespace NEXUS::UnitTests::NCore::FNMakeConvexPrismHarness;

	const FNRawMesh Prism = FNRawMeshUtils::MakeConvexPrism(MakeCap(0.0, 50.0), MakeCap(100.0, 20.0));

	CHECK_MESSAGE(TEXT("Prism bounds must span from the first cap to the second."),
		Prism.Bounds.Min.Equals(FVector(-50.0, 0.0, -50.0)) && Prism.Bounds.Max.Equals(FVector(50.0, 100.0, 50.0)))
}

N_TEST_CRITICAL(FNRawMeshUtilsTests_MakeConvexPrism_DetectsIntersectionAlongTheSweep,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::MakeConvexPrism::DetectsIntersectionAlongTheSweep",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The prism exists to be swept along a path and tested, so it must report geometry inside the volume it spans
	// and stay quiet about geometry beside it.
	using namespace NEXUS::UnitTests::NCore::FNMakeConvexPrismHarness;

	const FNRawMesh Prism = FNRawMeshUtils::MakeConvexPrism(MakeCap(0.0, 50.0), MakeCap(100.0, 50.0));

	const FNRawMesh Inside = FNRawMeshUtils::MakeBoxHull(FBox(FVector(-10.0, 40.0, -10.0), FVector(10.0, 60.0, 10.0)));
	CHECK_MESSAGE(TEXT("A box inside the swept volume must intersect the prism."),
		FNRawMeshUtils::DoesIntersect(Prism, Inside))

	const FNRawMesh Beside = FNRawMeshUtils::MakeBoxHull(FBox(FVector(200.0, 40.0, -10.0), FVector(300.0, 60.0, 10.0)));
	CHECK_FALSE_MESSAGE(TEXT("A box clear of the swept volume must not intersect the prism."),
		FNRawMeshUtils::DoesIntersect(Prism, Beside))

	const FNRawMesh PastTheEnd = FNRawMeshUtils::MakeBoxHull(FBox(FVector(-10.0, 200.0, -10.0), FVector(10.0, 300.0, 10.0)));
	CHECK_FALSE_MESSAGE(TEXT("A box past the end of the sweep must not intersect the prism."),
		FNRawMeshUtils::DoesIntersect(Prism, PastTheEnd))
}

N_TEST_MEDIUM(FNRawMeshUtilsTests_MakeConvexPrism_DegenerateCapsDoNotFail,
	"NEXUS::UnitTests::NCore::FNRawMeshUtils::MakeConvexPrism::DegenerateCapsDoNotFail",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A swept path can produce a zero-length segment where two samples coincide. That yields a degenerate solid,
	// which is fine — what matters is that it still emits well-formed topology rather than tripping over itself.
	using namespace NEXUS::UnitTests::NCore::FNMakeConvexPrismHarness;

	const TStaticArray<FVector, 4> Cap = MakeCap(0.0, 50.0);
	const FNRawMesh Prism = FNRawMeshUtils::MakeConvexPrism(Cap, Cap);

	CHECK_EQUALS("A degenerate prism must still emit 8 vertices.", Prism.Vertices.Num(), 8)
	CHECK_EQUALS("A degenerate prism must still emit 12 triangle loops.", Prism.Loops.Num(), 12)
	CHECK_MESSAGE(TEXT("A degenerate prism's bounds must collapse onto the shared cap."),
		Prism.Bounds.Min.Equals(FVector(-50.0, 0.0, -50.0)) && Prism.Bounds.Max.Equals(FVector(50.0, 0.0, 50.0)))
}

#endif //WITH_TESTS
