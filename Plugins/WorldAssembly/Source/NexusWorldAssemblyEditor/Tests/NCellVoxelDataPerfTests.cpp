// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Cell/NCellVoxelData.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NWorldAssembly::FNCellVoxelDataHarness
{
	// Cubic grid dimensions; voxel counts are Dim^3 (4,096 and 32,768).
	constexpr uint32 SmallDim = 16;
	constexpr uint32 LargeDim = 32;

	constexpr int32 SmallIterations = 1000;
	constexpr int32 LargeIterations = 100;

	// MaxDuration values are in milliseconds and bound the total cost of the inner loop (NOT per call).
	// Sized to roughly 5x the observed baseline so the tests don't false-fail under CI / contended cores
	// while still flagging genuine regressions.
	constexpr float CardinalSmallMaxDuration = 350.0f;
	constexpr float CardinalLargeMaxDuration = 300.0f;
	constexpr float ArbitrarySmallMaxDuration = 2800.0f;
	constexpr float ArbitraryLargeMaxDuration = 2200.0f;

	constexpr float VoxelExtent = 100.0f;

	/** Builds a Dim^3 grid with a checkerboard occupancy pattern so the cardinal write path does real work. */
	static FNCellVoxelData MakeGrid(const uint32 Dim)
	{
		FNCellVoxelData Grid;
		Grid.Resize(Dim, Dim, Dim);
		const uint32 Count = Dim * Dim * Dim;
		for (uint32 i = 0; i < Count; ++i)
		{
			if ((i & 1u) == 0u) Grid.SetData(i, static_cast<uint8>(ENCellVoxel::Occupied));
		}
		return Grid;
	}

	/** Pre-builds Count fresh grids so each timed rotation starts from an identical, un-rotated source. */
	static TArray<FNCellVoxelData> MakeGrids(const uint32 Dim, const int32 Count)
	{
		TArray<FNCellVoxelData> Grids;
		Grids.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			Grids.Add(MakeGrid(Dim));
		}
		return Grids;
	}

	/** @return The world-space center of a Dim^3 grid anchored at the origin, used as the rotation pivot. */
	static FVector GridCenter(const uint32 Dim)
	{
		return FVector(Dim * VoxelExtent * 0.5);
	}
}

class FNCellVoxelDataPerfTests
{
public:
	/** Cardinal 90-degree yaw on a 16^3 grid — exercises the exact integer forward-map (occupied voxels only). */
	static void RotatedAroundPivot_Cardinal_Small()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNCellVoxelDataHarness;
		TArray<FNCellVoxelData> Grids = MakeGrids(SmallDim, SmallIterations);
		const FVector Pivot = GridCenter(SmallDim);
		const FVector VoxelSize = FVector(VoxelExtent);
		const FRotator Rotation(0.0, 90.0, 0.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCellVoxelDataPerfTests_RotatedAroundPivot_Cardinal_Small, CardinalSmallMaxDuration)
			for (int32 i = 0; i < SmallIterations; ++i)
			{
				Grids[i].RotateAroundPivot(Pivot, Rotation, VoxelSize);
			}
			NTestTimer.ManualStop();
		}
	}

	/** Cardinal 90-degree yaw on a 32^3 grid — forward-map cost scaling with source volume. */
	static void RotatedAroundPivot_Cardinal_Large()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNCellVoxelDataHarness;
		TArray<FNCellVoxelData> Grids = MakeGrids(LargeDim, LargeIterations);
		const FVector Pivot = GridCenter(LargeDim);
		const FVector VoxelSize = FVector(VoxelExtent);
		const FRotator Rotation(0.0, 90.0, 0.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCellVoxelDataPerfTests_RotatedAroundPivot_Cardinal_Large, CardinalLargeMaxDuration)
			for (int32 i = 0; i < LargeIterations; ++i)
			{
				Grids[i].RotateAroundPivot(Pivot, Rotation, VoxelSize);
			}
			NTestTimer.ManualStop();
		}
	}

	/** Arbitrary 3-axis rotation on a 16^3 grid — drives the inverse-sampling path over the grown destination volume. */
	static void RotatedAroundPivot_Arbitrary_Small()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNCellVoxelDataHarness;
		TArray<FNCellVoxelData> Grids = MakeGrids(SmallDim, SmallIterations);
		const FVector Pivot = GridCenter(SmallDim);
		const FVector VoxelSize = FVector(VoxelExtent);
		const FRotator Rotation(15.0, 37.0, 30.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCellVoxelDataPerfTests_RotatedAroundPivot_Arbitrary_Small, ArbitrarySmallMaxDuration)
			for (int32 i = 0; i < SmallIterations; ++i)
			{
				Grids[i].RotateAroundPivot(Pivot, Rotation, VoxelSize);
			}
			NTestTimer.ManualStop();
		}
	}

	/** Arbitrary 3-axis rotation on a 32^3 grid — inverse-sampling worst case (destination AABB ~sqrt(3)x larger). */
	static void RotatedAroundPivot_Arbitrary_Large()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNCellVoxelDataHarness;
		TArray<FNCellVoxelData> Grids = MakeGrids(LargeDim, LargeIterations);
		const FVector Pivot = GridCenter(LargeDim);
		const FVector VoxelSize = FVector(VoxelExtent);
		const FRotator Rotation(15.0, 37.0, 30.0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCellVoxelDataPerfTests_RotatedAroundPivot_Arbitrary_Large, ArbitraryLargeMaxDuration)
			for (int32 i = 0; i < LargeIterations; ++i)
			{
				Grids[i].RotateAroundPivot(Pivot, Rotation, VoxelSize);
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNCellVoxelDataPerfTests_RotatedAroundPivot_Cardinal_Small,
	"NEXUS::PerfTests::NWorldAssembly::FNCellVoxelData::RotatedAroundPivot_Cardinal_Small",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCellVoxelDataPerfTests::RotatedAroundPivot_Cardinal_Small));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCellVoxelDataPerfTests_RotatedAroundPivot_Cardinal_Large,
	"NEXUS::PerfTests::NWorldAssembly::FNCellVoxelData::RotatedAroundPivot_Cardinal_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCellVoxelDataPerfTests::RotatedAroundPivot_Cardinal_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCellVoxelDataPerfTests_RotatedAroundPivot_Arbitrary_Small,
	"NEXUS::PerfTests::NWorldAssembly::FNCellVoxelData::RotatedAroundPivot_Arbitrary_Small",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCellVoxelDataPerfTests::RotatedAroundPivot_Arbitrary_Small));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCellVoxelDataPerfTests_RotatedAroundPivot_Arbitrary_Large,
	"NEXUS::PerfTests::NWorldAssembly::FNCellVoxelData::RotatedAroundPivot_Arbitrary_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCellVoxelDataPerfTests::RotatedAroundPivot_Arbitrary_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
