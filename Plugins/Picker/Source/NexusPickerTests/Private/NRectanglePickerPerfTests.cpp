// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NRectanglePicker.h"
#include "NRectanglePickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNRectanglePickerHarness
{
	constexpr float MaxDuration = 0.2f;
}

class FNRectanglePickerPerfTests
{
public:
	static void NextSimple()
	{
		FNRectanglePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MaximumDimensions = FVector2D(100, 100);
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNRectanglePickerHarness::MaxDuration)
			FNRectanglePicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Next()
	{
		FNRectanglePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumDimensions = FVector2D(1, 1);
		Params.MaximumDimensions = FVector2D(100, 100);
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNRectanglePickerHarness::MaxDuration)
			FNRectanglePicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void RandomSimple()
	{
		FNRectanglePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MaximumDimensions = FVector2D(100, 100);
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNRectanglePickerHarness::MaxDuration)
			FNRectanglePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Random()
	{
		FNRectanglePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumDimensions = FVector2D(1, 1);
		Params.MaximumDimensions = FVector2D(100, 100);
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNRectanglePickerHarness::MaxDuration)
			FNRectanglePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void TrackedSimple()
	{
		FNRectanglePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MaximumDimensions = FVector2D(100, 100);
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNRectanglePickerHarness::MaxDuration)
			FNRectanglePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Tracked()
	{
		FNRectanglePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumDimensions = FVector2D(1, 1);
		Params.MaximumDimensions = FVector2D(100, 100);
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNRectanglePickerHarness::MaxDuration)
			FNRectanglePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNRectanglePickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNRectanglePicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRectanglePickerPerfTests::NextSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRectanglePickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNRectanglePicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRectanglePickerPerfTests::Next));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRectanglePickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNRectanglePicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRectanglePickerPerfTests::RandomSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRectanglePickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNRectanglePicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRectanglePickerPerfTests::Random));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRectanglePickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNRectanglePicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRectanglePickerPerfTests::TrackedSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNRectanglePickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNRectanglePicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNRectanglePickerPerfTests::Tracked));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
