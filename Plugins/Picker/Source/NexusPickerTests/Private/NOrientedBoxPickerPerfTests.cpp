// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NOrientedBoxPicker.h"
#include "NOrientedBoxPickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness
{
	constexpr float MaxDuration = 0.2f;
}

class FNOrientedBoxPickerPerfTests
{
public:
	static void NextSimple()
	{
		FNOrientedBoxPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MaximumDimensions = FVector(100.f);
		Params.Rotation = FRotator::ZeroRotator;
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
			FNOrientedBoxPicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Next()
	{
		FNOrientedBoxPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumDimensions = FVector(10.f);
		Params.MaximumDimensions = FVector(100.f);
		Params.Rotation = FRotator(0.f, 45.f, 0.f);
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
			FNOrientedBoxPicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void RandomSimple()
	{
		FNOrientedBoxPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MaximumDimensions = FVector(100.f);
		Params.Rotation = FRotator::ZeroRotator;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
			FNOrientedBoxPicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Random()
	{
		FNOrientedBoxPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumDimensions = FVector(10.f);
		Params.MaximumDimensions = FVector(100.f);
		Params.Rotation = FRotator(0.f, 45.f, 0.f);
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
			FNOrientedBoxPicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void TrackedSimple()
	{
		FNOrientedBoxPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MaximumDimensions = FVector(100.f);
		Params.Rotation = FRotator::ZeroRotator;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
			FNOrientedBoxPicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Tracked()
	{
		FNOrientedBoxPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumDimensions = FVector(10.f);
		Params.MaximumDimensions = FVector(100.f);
		Params.Rotation = FRotator(0.f, 45.f, 0.f);
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
			FNOrientedBoxPicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNOrientedBoxPickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrientedBoxPickerPerfTests::NextSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrientedBoxPickerPerfTests::Next));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrientedBoxPickerPerfTests::RandomSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrientedBoxPickerPerfTests::Random));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrientedBoxPickerPerfTests::TrackedSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrientedBoxPickerPerfTests::Tracked));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
