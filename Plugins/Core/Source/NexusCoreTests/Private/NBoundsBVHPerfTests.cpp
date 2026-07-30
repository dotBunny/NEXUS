// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NBoundsBVH.h"
#include "Math/NMersenneTwister.h"
#include "Macros/NTestMacros.h"

/**
 * Query and build cost for FNBoundsBVH, sized to line up with the graph-builder collision baseline in
 * NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask so the two are directly comparable: the same 100 /
 * 1000 / 5000 entry counts, the same miss-dominated query pattern.
 *
 * The brute-force counterpart is measured alongside the tree at the largest size. That pairing is the point —
 * it states the win in one place rather than leaving it to be inferred across two suites.
 */
namespace NEXUS::PerfTests::NCore::FNBoundsBVHHarness
{
	constexpr int32 SmallCount = 100;
	constexpr int32 MediumCount = 1000;
	constexpr int32 LargeCount = 5000;

	constexpr int32 QueryIterations = 10000;
	constexpr int32 BuildIterations = 200;

	// MaxDuration values are in milliseconds and bound the total cost of the inner loop (NOT per call), sized to
	// roughly 4x the observed baseline.
	//
	// The three query gates are proportionally looser than that. The tree answers a query in ~0.13us, so even
	// 10,000 of them land near a millisecond, and a sub-millisecond measurement carries enough timer jitter that
	// a 4x gate would false-fail on a loaded machine. Raising the iteration count until they were stable would
	// have pushed the brute-force comparison below into the multi-second range for no extra information. The
	// speedup this file exists to demonstrate is read from the Large query / brute-force pair, which share an
	// iteration count and are therefore directly comparable.
	constexpr float SmallQueryMaxDuration = 6.0f;
	constexpr float MediumQueryMaxDuration = 6.0f;
	constexpr float LargeQueryMaxDuration = 10.0f;
	constexpr float LargeBruteForceMaxDuration = 600.0f;
	constexpr float LargeBuildMaxDuration = 400.0f;

	/** @return A deterministic scatter of boxes over a fixed volume, so density rises with Count. */
	static TArray<FBox> MakeScatteredBounds(const int32 Count, const uint64 Seed)
	{
		FNMersenneTwister Random(Seed);
		TArray<FBox> Bounds;
		Bounds.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			const FVector Center(
				Random.DoubleRange(-5000.0, 5000.0),
				Random.DoubleRange(-5000.0, 5000.0),
				Random.DoubleRange(-5000.0, 5000.0));
			const FVector Extent(
				Random.DoubleRange(25.0, 75.0),
				Random.DoubleRange(25.0, 75.0),
				Random.DoubleRange(25.0, 75.0));
			Bounds.Add(FBox(Center - Extent, Center + Extent));
		}
		return Bounds;
	}

	/** @return Deterministic query boxes the size of a cell hull, scattered over the same volume as the entries. */
	static TArray<FBox> MakeQueryBoxes(const int32 Count, const uint64 Seed)
	{
		FNMersenneTwister Random(Seed);
		TArray<FBox> Queries;
		Queries.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			const FVector Center(
				Random.DoubleRange(-5000.0, 5000.0),
				Random.DoubleRange(-5000.0, 5000.0),
				Random.DoubleRange(-5000.0, 5000.0));
			Queries.Add(FBox(Center - FVector(50.0), Center + FVector(50.0)));
		}
		return Queries;
	}
}

class FNBoundsBVHPerfTests
{
public:
	static void QueryOverlaps_Small()
	{
		using namespace NEXUS::PerfTests::NCore::FNBoundsBVHHarness;
		const TArray<FBox> Bounds = MakeScatteredBounds(SmallCount, 1ull);
		const TArray<FBox> Queries = MakeQueryBoxes(256, 2ull);
		const FNBoundsBVH BVH(Bounds);
		TArray<int32> Scratch;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNBoundsBVHPerfTests_QueryOverlaps_Small, SmallQueryMaxDuration)
			for (int32 i = 0; i < QueryIterations; ++i)
			{
				BVH.QueryOverlaps(Queries[i % Queries.Num()], Scratch);
			}
			NTestTimer.ManualStop();
		}
	}

	static void QueryOverlaps_Medium()
	{
		using namespace NEXUS::PerfTests::NCore::FNBoundsBVHHarness;
		const TArray<FBox> Bounds = MakeScatteredBounds(MediumCount, 1ull);
		const TArray<FBox> Queries = MakeQueryBoxes(256, 2ull);
		const FNBoundsBVH BVH(Bounds);
		TArray<int32> Scratch;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNBoundsBVHPerfTests_QueryOverlaps_Medium, MediumQueryMaxDuration)
			for (int32 i = 0; i < QueryIterations; ++i)
			{
				BVH.QueryOverlaps(Queries[i % Queries.Num()], Scratch);
			}
			NTestTimer.ManualStop();
		}
	}

	/** The size that matters: 5000 entries, matching the dense-level figure in the graph-builder baseline. */
	static void QueryOverlaps_Large()
	{
		using namespace NEXUS::PerfTests::NCore::FNBoundsBVHHarness;
		const TArray<FBox> Bounds = MakeScatteredBounds(LargeCount, 1ull);
		const TArray<FBox> Queries = MakeQueryBoxes(256, 2ull);
		const FNBoundsBVH BVH(Bounds);
		TArray<int32> Scratch;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNBoundsBVHPerfTests_QueryOverlaps_Large, LargeQueryMaxDuration)
			for (int32 i = 0; i < QueryIterations; ++i)
			{
				BVH.QueryOverlaps(Queries[i % Queries.Num()], Scratch);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * The linear sweep the tree replaces, over the same 5000 entries and the same queries. Runs the identical
	 * FBox::Intersect test per entry, so the ratio against QueryOverlaps_Large is the broadphase's speedup with
	 * nothing else varying.
	 */
	static void BruteForceOverlaps_Large()
	{
		using namespace NEXUS::PerfTests::NCore::FNBoundsBVHHarness;
		const TArray<FBox> Bounds = MakeScatteredBounds(LargeCount, 1ull);
		const TArray<FBox> Queries = MakeQueryBoxes(256, 2ull);
		TArray<int32> Scratch;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNBoundsBVHPerfTests_BruteForceOverlaps_Large, LargeBruteForceMaxDuration)
			for (int32 i = 0; i < QueryIterations; ++i)
			{
				const FBox& QueryBox = Queries[i % Queries.Num()];
				Scratch.Reset();
				for (int32 b = 0; b < Bounds.Num(); ++b)
				{
					if (Bounds[b].Intersect(QueryBox))
					{
						Scratch.Add(b);
					}
				}
			}
			NTestTimer.ManualStop();
		}
	}

	/** Build cost at the largest size — paid once per pass, so it must stay small against the queries it saves. */
	static void Build_Large()
	{
		using namespace NEXUS::PerfTests::NCore::FNBoundsBVHHarness;
		const TArray<FBox> Bounds = MakeScatteredBounds(LargeCount, 1ull);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNBoundsBVHPerfTests_Build_Large, LargeBuildMaxDuration)
			for (int32 i = 0; i < BuildIterations; ++i)
			{
				const FNBoundsBVH BVH(Bounds);
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNBoundsBVHPerfTests_QueryOverlaps_Small,
	"NEXUS::PerfTests::NCore::FNBoundsBVH::QueryOverlaps_Small",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNBoundsBVHPerfTests::QueryOverlaps_Small));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNBoundsBVHPerfTests_QueryOverlaps_Medium,
	"NEXUS::PerfTests::NCore::FNBoundsBVH::QueryOverlaps_Medium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNBoundsBVHPerfTests::QueryOverlaps_Medium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNBoundsBVHPerfTests_QueryOverlaps_Large,
	"NEXUS::PerfTests::NCore::FNBoundsBVH::QueryOverlaps_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNBoundsBVHPerfTests::QueryOverlaps_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNBoundsBVHPerfTests_BruteForceOverlaps_Large,
	"NEXUS::PerfTests::NCore::FNBoundsBVH::BruteForceOverlaps_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNBoundsBVHPerfTests::BruteForceOverlaps_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNBoundsBVHPerfTests_Build_Large,
	"NEXUS::PerfTests::NCore::FNBoundsBVH::Build_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNBoundsBVHPerfTests::Build_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
