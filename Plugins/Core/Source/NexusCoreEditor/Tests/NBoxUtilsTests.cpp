// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NBoxUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNBoxUtilsTests_GetVertices_Count,
	"NEXUS::UnitTests::NCore::FNBoxUtils::GetVertices::Count",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FBox Box(FVector(-1, -2, -3), FVector(4, 5, 6));
	const TArray<FVector> Vertices = FNBoxUtils::GetVertices(Box);
	CHECK_EQUALS("A box must produce exactly 8 corner vertices", Vertices.Num(), 8);
}

N_TEST_HIGH(FNBoxUtilsTests_GetVertices_AllCornersPresent,
	"NEXUS::UnitTests::NCore::FNBoxUtils::GetVertices::AllCornersPresent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Order-independent check that every (Min/Max permutation) corner shows up in the result. Locks the
	// "all 8 corners" guarantee without coupling to the emitter's iteration order — callers like
	// FNRawMeshFactory::FromChaosBox depend on completeness, not order.
	const FBox Box(FVector(-1, -2, -3), FVector(4, 5, 6));
	const TArray<FVector> Vertices = FNBoxUtils::GetVertices(Box);

	const FVector Expected[8] = {
		FVector(-1, -2, -3), FVector(-1, -2,  6), FVector( 4, -2,  6), FVector( 4, -2, -3),
		FVector(-1,  5, -3), FVector(-1,  5,  6), FVector( 4,  5,  6), FVector( 4,  5, -3),
	};
	for (int32 i = 0; i < 8; i++)
	{
		bool bFound = false;
		for (const FVector& V : Vertices)
		{
			if (V.Equals(Expected[i], 0.001))
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			ADD_ERROR("Expected corner was not present in GetVertices output");
		}
	}
}

N_TEST_HIGH(FNBoxUtilsTests_GetVertices_DegenerateBox_PointVertex,
	"NEXUS::UnitTests::NCore::FNBoxUtils::GetVertices::DegenerateBox_PointVertex",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Min==Max — the box collapses to a single point. GetVertices still emits 8 entries, all equal to
	// that point. Documents the unconditional 8-vertex contract.
	const FBox Box(FVector(7, 7, 7), FVector(7, 7, 7));
	const TArray<FVector> Vertices = FNBoxUtils::GetVertices(Box);

	CHECK_EQUALS("Degenerate point-box must still emit 8 vertices", Vertices.Num(), 8);
	for (const FVector& V : Vertices)
	{
		CHECK_MESSAGE(TEXT("Every emitted vertex must equal the collapsed point"), V.Equals(FVector(7, 7, 7), 0.001));
	}
}

N_TEST_HIGH(FNBoxUtilsTests_GetVertices_UnitCube_BoundsAndCenter,
	"NEXUS::UnitTests::NCore::FNBoxUtils::GetVertices::UnitCube_BoundsAndCenter",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Reconstruct an FBox from the emitted vertices and confirm it matches the source — proves the corners
	// span the box exactly (no off-by-one along any axis).
	const FBox Source(FVector(-5, -5, -5), FVector(5, 5, 5));
	const TArray<FVector> Vertices = FNBoxUtils::GetVertices(Source);

	FBox Rebuilt(ForceInit);
	for (const FVector& V : Vertices)
	{
		Rebuilt += V;
	}

	CHECK_MESSAGE(TEXT("Bounds rebuilt from the emitted vertices must match the source minimum"),
		Rebuilt.Min.Equals(Source.Min, 0.001));
	CHECK_MESSAGE(TEXT("Bounds rebuilt from the emitted vertices must match the source maximum"),
		Rebuilt.Max.Equals(Source.Max, 0.001));
}

#endif //WITH_TESTS
