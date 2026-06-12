// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArcPicker.h"
#include "NArcPickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNArcPickerHarness
{
	constexpr float MaxDuration = 0.2f;
}

class FNArcPickerPerfTests
{
public:
	static void NextSimple()
	{
		FNArcPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.Rotation = FRotator::ZeroRotator;
		Params.Degrees = 90.f;
		Params.MinimumDistance = 0.f;
		Params.MaximumDistance = 500.f;
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
			FNArcPicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Next()
	{
		FNArcPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.Rotation = FRotator::ZeroRotator;
		Params.Degrees = 180.f;
		Params.MinimumDistance = 10.f;
		Params.MaximumDistance = 500.f;
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
			FNArcPicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}
	
	static void RandomSimple()
	{
		FNArcPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.Rotation = FRotator::ZeroRotator;
		Params.Degrees = 90.f;
		Params.MinimumDistance = 0.f;
		Params.MaximumDistance = 500.f;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
			FNArcPicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}
	
	static void Random()
	{
		FNArcPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.Rotation = FRotator::ZeroRotator;
		Params.Degrees = 180.f;
		Params.MinimumDistance = 10.f;
		Params.MaximumDistance = 500.f;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
			FNArcPicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}
	
	static void TrackedSimple()
	{
		FNArcPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.Rotation = FRotator::ZeroRotator;
		Params.Degrees = 90.f;
		Params.MinimumDistance = 0.f;
		Params.MaximumDistance = 500.f;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
			FNArcPicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
	
	static void Tracked()
	{
		FNArcPickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.Rotation = FRotator::ZeroRotator;
		Params.Degrees = 180.f;
		Params.MinimumDistance = 10.f;
		Params.MaximumDistance = 500.f;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
			FNArcPicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNArcPickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNArcPicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNArcPickerPerfTests::NextSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNArcPickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNArcPicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNArcPickerPerfTests::Next));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNArcPickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNArcPicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNArcPickerPerfTests::RandomSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNArcPickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNArcPicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNArcPickerPerfTests::Random));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNArcPickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNArcPicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNArcPickerPerfTests::TrackedSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNArcPickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNArcPicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNArcPickerPerfTests::Tracked));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
