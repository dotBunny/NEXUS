// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Cell/NCellJunctionDetails.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Macros/NTestMacros.h"

/**
 * Cost of FNVirtualOrganContext::FilterCellInputData, which narrows the tissue's cell pool to the candidates that
 * could fill one particular open junction.
 *
 * It runs once per junction-fill attempt — so once per open junction in ProcessCellNode, again for the start-node
 * pre-filter, and again in the finisher passes — and each call is O(cells in the pool x junctions per cell). That
 * makes it a per-placement cost like the collision scans, and it needs to be read against the same denominator:
 * ~1.7us to construct and discard a candidate node.
 */
namespace NEXUS::PerfTests::NWorldAssembly::FNFilterCellInputDataHarness
{
	/** Cell-pool sizes standing in for a modest and a large tissue. */
	constexpr int32 SmallPoolSize = 32;
	constexpr int32 LargePoolSize = 128;

	/** Junctions per cell — one per face of a box cell. */
	constexpr int32 JunctionsPerCell = 6;

	constexpr int32 Iterations = 10000;

	// MaxDuration values are in milliseconds and bound the total cost of the inner loop (NOT per call).
	constexpr float SmallPoolMaxDuration = 130.0f;
	constexpr float LargePoolMaxDuration = 500.0f;

	/** The socket size every junction and filter shares, so no candidate is rejected on socket mismatch. */
	static FIntVector2 MatchingSocket() { return FIntVector2(2, 4); }

	/**
	 * @return A cell carrying JunctionsPerCell matching junctions.
	 * @note Every junction matches the filter socket, so the inner junction loop runs to completion for every
	 *       candidate. That is the worst case, and also the realistic one for a tissue whose cells share a socket.
	 */
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.Weighting = 1;

		static const FVector FaceNormals[6] = {
			FVector(1, 0, 0), FVector(-1, 0, 0), FVector(0, 1, 0),
			FVector(0, -1, 0), FVector(0, 0, 1), FVector(0, 0, -1)
		};
		for (int32 i = 0; i < JunctionsPerCell; ++i)
		{
			FNCellJunctionDetails Junction;
			Junction.SocketSize = MatchingSocket();
			Junction.WorldLocation = FaceNormals[i] * 100.0;
			Junction.WorldRotation = FaceNormals[i].Rotation();
			Junction.CachedInverseWorldQuat = Junction.WorldRotation.Quaternion().Inverse();
			Cell.Junctions.Add(i, Junction);
		}
		return Cell;
	}

	static FNCellInputDataFilter MakeFilter()
	{
		FNCellInputDataFilter Filter;
		Filter.SocketSize = MatchingSocket();
		Filter.SourceQuat = FQuat::Identity;
		Filter.WorldPosition = FVector(500.0, 0.0, 0.0);
		return Filter;
	}
}

class FNFilterCellInputDataPerfTests
{
public:
	static void FilterCellInputData_SmallPool()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNFilterCellInputDataHarness;

		FNVirtualOrganContext Context(1234ull, TEXT("FilterPerf"));
		for (int32 i = 0; i < SmallPoolSize; ++i)
		{
			Context.CellInputData.Add(MakeCell());
		}
		const FNCellInputDataFilter Filter = MakeFilter();

		// Reused across calls, as the builder reuses them, so the measurement is the filter and not array churn.
		FNWeightedIntegerArray CellIndices;
		TMap<int32, TArray<int32>> JunctionIndices;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNVirtualOrganContext_FilterCellInputData_SmallPool, SmallPoolMaxDuration)
			for (int32 i = 0; i < Iterations; ++i)
			{
				Context.FilterCellInputData(Filter, CellIndices, JunctionIndices);
			}
			NTestTimer.ManualStop();
		}
	}

	static void FilterCellInputData_LargePool()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNFilterCellInputDataHarness;

		FNVirtualOrganContext Context(1234ull, TEXT("FilterPerf"));
		for (int32 i = 0; i < LargePoolSize; ++i)
		{
			Context.CellInputData.Add(MakeCell());
		}
		const FNCellInputDataFilter Filter = MakeFilter();

		FNWeightedIntegerArray CellIndices;
		TMap<int32, TArray<int32>> JunctionIndices;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNVirtualOrganContext_FilterCellInputData_LargePool, LargePoolMaxDuration)
			for (int32 i = 0; i < Iterations; ++i)
			{
				Context.FilterCellInputData(Filter, CellIndices, JunctionIndices);
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNFilterCellInputDataPerfTests_SmallPool,
	"NEXUS::PerfTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData_SmallPool",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNFilterCellInputDataPerfTests::FilterCellInputData_SmallPool));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNFilterCellInputDataPerfTests_LargePool,
	"NEXUS::PerfTests::NWorldAssembly::FNVirtualOrganContext::FilterCellInputData_LargePool",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNFilterCellInputDataPerfTests::FilterCellInputData_LargePool));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
