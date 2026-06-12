// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NCirclePicker.h"
#include "NCirclePickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNCirclePickerHarness
{
	constexpr float MaxDuration = 0.2f;
}

class FNCirclePickerPerfTests
{
public:
	static void NextSimple()
	{
		FNCirclePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNCirclePickerHarness::MaxDuration)
			FNCirclePicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Next()
	{
		FNCirclePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		Params.Rotation = FRotator(10.4);
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNCirclePickerHarness::MaxDuration)
			FNCirclePicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void RandomSimple()
	{
		FNCirclePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNCirclePickerHarness::MaxDuration)
			FNCirclePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Random()
	{
		FNCirclePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		Params.Rotation = FRotator(10.4);
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNCirclePickerHarness::MaxDuration)
			FNCirclePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void TrackedSimple()
	{
		FNCirclePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNCirclePickerHarness::MaxDuration)
			FNCirclePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Tracked()
	{
		FNCirclePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		Params.Rotation = FRotator(10.4);
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNCirclePickerHarness::MaxDuration)
			FNCirclePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNCirclePickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNCirclePicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCirclePickerPerfTests::NextSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCirclePickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNCirclePicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCirclePickerPerfTests::Next));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCirclePickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNCirclePicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCirclePickerPerfTests::RandomSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCirclePickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNCirclePicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCirclePickerPerfTests::Random));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCirclePickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNCirclePicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCirclePickerPerfTests::TrackedSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNCirclePickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNCirclePicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNCirclePickerPerfTests::Tracked));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
