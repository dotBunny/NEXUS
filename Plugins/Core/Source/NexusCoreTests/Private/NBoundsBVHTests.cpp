// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NBoundsBVH.h"
#include "Math/NMersenneTwister.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness
{
	/** @return Every index in Bounds whose box overlaps QueryBox, ascending — the definition the BVH must reproduce. */
	static TArray<int32> BruteForceOverlaps(const TArray<FBox>& Bounds, const FBox& QueryBox)
	{
		TArray<int32> Result;
		if (!QueryBox.IsValid)
		{
			return Result;
		}
		for (int32 i = 0; i < Bounds.Num(); ++i)
		{
			if (Bounds[i].IsValid && Bounds[i].Intersect(QueryBox))
			{
				Result.Add(i);
			}
		}
		return Result;
	}

	/** @return The BVH's answer for QueryBox, sorted ascending so it can be compared against the brute-force order. */
	static TArray<int32> SortedQuery(const FNBoundsBVH& BVH, const FBox& QueryBox)
	{
		TArray<int32> Result;
		BVH.QueryOverlaps(QueryBox, Result);
		Result.Sort();
		return Result;
	}

	/** @return A deterministic scatter of axis-aligned boxes with varied positions and sizes. */
	static TArray<FBox> MakeScatteredBounds(const int32 Count, const uint64 Seed)
	{
		FNMersenneTwister Random(Seed);
		TArray<FBox> Bounds;
		Bounds.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			const FVector Center(
				Random.DoubleRange(-1000.0, 1000.0),
				Random.DoubleRange(-1000.0, 1000.0),
				Random.DoubleRange(-1000.0, 1000.0));
			// Sizes overlap heavily at the large end so queries return non-trivial result sets.
			const FVector Extent(
				Random.DoubleRange(10.0, 130.0),
				Random.DoubleRange(10.0, 130.0),
				Random.DoubleRange(10.0, 130.0));
			Bounds.Add(FBox(Center - Extent, Center + Extent));
		}
		return Bounds;
	}
}

N_TEST_HIGH(FNBoundsBVHTests_QueryOverlaps_MatchesBruteForce,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::QueryOverlaps::MatchesBruteForce",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The accelerator only earns its keep if it is an exact drop-in for the sweep it replaces, so compare the full
	// result set — not just its size — across many query boxes covering hits, misses and everything between.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const TArray<FBox> Bounds = MakeScatteredBounds(400, 20260727ull);
	const FNBoundsBVH BVH(Bounds);

	CHECK_EQUALS("Every scattered box is valid, so all of them should be indexed.", BVH.Num(), Bounds.Num())

	FNMersenneTwister Random(99ull);
	int32 NonEmptyResults = 0;
	for (int32 Query = 0; Query < 200; ++Query)
	{
		const FVector Center(
			Random.DoubleRange(-1200.0, 1200.0),
			Random.DoubleRange(-1200.0, 1200.0),
			Random.DoubleRange(-1200.0, 1200.0));
		const FVector Extent(
			Random.DoubleRange(0.0, 200.0),
			Random.DoubleRange(0.0, 200.0),
			Random.DoubleRange(0.0, 200.0));
		const FBox QueryBox(Center - Extent, Center + Extent);

		const TArray<int32> Expected = BruteForceOverlaps(Bounds, QueryBox);
		if (Expected.Num() > 0)
		{
			++NonEmptyResults;
		}

		if (SortedQuery(BVH, QueryBox) != Expected)
		{
			ADD_ERROR(FString::Printf(TEXT("Query %d disagreed with the brute-force sweep (expected %d overlaps)."),
				Query, Expected.Num()));
			break;
		}
	}

	CHECK_MESSAGE(TEXT("The query set must actually produce hits, or the comparison proves nothing."), NonEmptyResults > 0)
}

N_TEST_HIGH(FNBoundsBVHTests_QueryOverlaps_TouchingAndContained,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::QueryOverlaps::TouchingAndContained",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The boundary cases a hand-written sweep gets wrong: a query that only shares a face, one fully inside an
	// entry, and one fully containing it. All three must agree with FBox::Intersect.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const TArray<FBox> Bounds = { FBox(FVector(0.0), FVector(100.0)) };
	const FNBoundsBVH BVH(Bounds);

	const FBox FaceTouching(FVector(100.0, 0.0, 0.0), FVector(200.0, 100.0, 100.0));
	CHECK_EQUALS("A face-touching query must agree with FBox::Intersect.",
		SortedQuery(BVH, FaceTouching), BruteForceOverlaps(Bounds, FaceTouching))

	const FBox FullyInside(FVector(40.0), FVector(60.0));
	CHECK_EQUALS("A query contained by the entry overlaps it.", SortedQuery(BVH, FullyInside).Num(), 1)

	const FBox FullyContaining(FVector(-500.0), FVector(500.0));
	CHECK_EQUALS("A query containing the entry overlaps it.", SortedQuery(BVH, FullyContaining).Num(), 1)

	const FBox Separated(FVector(500.0), FVector(600.0));
	CHECK_EQUALS("A clearly separated query overlaps nothing.", SortedQuery(BVH, Separated).Num(), 0)
}

N_TEST_HIGH(FNBoundsBVHTests_Build_SkipsInvalidBoundsAndKeepsIndices,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::Build::SkipsInvalidBoundsAndKeepsIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Invalid entries are dropped, but the survivors must still report the index they had in the source array —
	// otherwise a caller's parallel arrays would silently desynchronise.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const TArray<FBox> Bounds = {
		FBox(ForceInit),                                   // 0: invalid
		FBox(FVector(0.0), FVector(10.0)),                 // 1
		FBox(ForceInit),                                   // 2: invalid
		FBox(FVector(5.0), FVector(15.0)),                 // 3
	};
	const FNBoundsBVH BVH(Bounds);

	CHECK_EQUALS("Only the two valid entries should be indexed.", BVH.Num(), 2)

	const FBox QueryBox(FVector(-1.0), FVector(20.0));
	const TArray<int32> Expected = { 1, 3 };
	CHECK_EQUALS("Surviving entries must report their original source indices.", SortedQuery(BVH, QueryBox), Expected)
}

N_TEST_HIGH(FNBoundsBVHTests_QueryOverlaps_CoincidentBounds,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::QueryOverlaps::CoincidentBounds",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Identical bounds give every centroid the same position, which defeats the centroid split and drives the
	// build's degenerate-split fallback. All of them must still be reachable.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	TArray<FBox> Bounds;
	for (int32 i = 0; i < 32; ++i)
	{
		Bounds.Add(FBox(FVector(0.0), FVector(50.0)));
	}
	const FNBoundsBVH BVH(Bounds);

	CHECK_EQUALS("Coincident entries are still all indexed.", BVH.Num(), Bounds.Num())
	CHECK_EQUALS("A query over the shared box returns every coincident entry.",
		SortedQuery(BVH, FBox(FVector(10.0), FVector(20.0))), BruteForceOverlaps(Bounds, FBox(FVector(10.0), FVector(20.0))))
}

N_TEST_MEDIUM(FNBoundsBVHTests_Empty_ReturnsNoOverlaps,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::Empty::ReturnsNoOverlaps",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A default-constructed tree, and one built from nothing but invalid bounds, must both answer emptily rather
	// than indexing into an empty node pool.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const FNBoundsBVH Default;
	CHECK_MESSAGE(TEXT("A default-constructed hierarchy is empty."), Default.IsEmpty())
	CHECK_EQUALS("An empty hierarchy overlaps nothing.", SortedQuery(Default, FBox(FVector(0.0), FVector(1.0))).Num(), 0)
	CHECK_FALSE_MESSAGE(TEXT("An empty hierarchy reports invalid bounds."), Default.GetBounds().IsValid)

	const TArray<FBox> AllInvalid = { FBox(ForceInit), FBox(ForceInit) };
	const FNBoundsBVH Invalid(AllInvalid);
	CHECK_MESSAGE(TEXT("A hierarchy of only-invalid bounds is empty."), Invalid.IsEmpty())
	CHECK_EQUALS("It overlaps nothing.", SortedQuery(Invalid, FBox(FVector(0.0), FVector(1.0))).Num(), 0)
}

N_TEST_MEDIUM(FNBoundsBVHTests_QueryOverlaps_InvalidQueryMatchesNothing,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::QueryOverlaps::InvalidQueryMatchesNothing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An invalid query box is not a wildcard — it must match nothing, matching FBox::Intersect's own answer.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const TArray<FBox> Bounds = MakeScatteredBounds(16, 7ull);
	const FNBoundsBVH BVH(Bounds);

	CHECK_EQUALS("An invalid query box matches nothing.", SortedQuery(BVH, FBox(ForceInit)).Num(), 0)
}

N_TEST_MEDIUM(FNBoundsBVHTests_QueryOverlaps_ReusesCallerArray,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::QueryOverlaps::ReusesCallerArray",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The contract that makes this cheap to call in a loop: the result array is Reset (keeping its allocation),
	// never appended to, so a stale result from a previous query can never leak into the next one.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const TArray<FBox> Bounds = { FBox(FVector(0.0), FVector(10.0)) };
	const FNBoundsBVH BVH(Bounds);

	TArray<int32> Results;
	BVH.QueryOverlaps(FBox(FVector(1.0), FVector(2.0)), Results);
	CHECK_EQUALS("The hit query reports the single entry.", Results.Num(), 1)

	const int32 CapacityAfterHit = Results.Max();

	BVH.QueryOverlaps(FBox(FVector(500.0), FVector(600.0)), Results);
	CHECK_EQUALS("A subsequent miss clears the previous result.", Results.Num(), 0)
	CHECK_EQUALS("Clearing keeps the array's allocation for the next query.", Results.Max(), CapacityAfterHit)
}

N_TEST_MEDIUM(FNBoundsBVHTests_GetBounds_EnclosesEveryEntry,
	"NEXUS::UnitTests::NCore::FNBoundsBVH::GetBounds::EnclosesEveryEntry",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The root box is what the first prune tests against, so it must genuinely enclose everything.
	using namespace NEXUS::UnitTests::NCore::FNBoundsBVHHarness;

	const TArray<FBox> Bounds = MakeScatteredBounds(64, 31ull);
	const FNBoundsBVH BVH(Bounds);

	FBox Expected(ForceInit);
	for (const FBox& Box : Bounds)
	{
		Expected += Box;
	}

	CHECK_MESSAGE(TEXT("The root bounds enclose the union of every entry."),
		BVH.GetBounds().Min.Equals(Expected.Min) && BVH.GetBounds().Max.Equals(Expected.Max))
}

#endif //WITH_TESTS
