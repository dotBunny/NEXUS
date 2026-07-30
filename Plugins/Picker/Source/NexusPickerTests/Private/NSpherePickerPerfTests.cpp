// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NSpherePicker.h"
#include "NSpherePickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNSpherePickerHarness
{
	constexpr float MaxDuration = 0.2f;
}

class FNSpherePickerPerfTests
{
public:
	static void Next()
	{
		FNSpherePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSpherePickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNSpherePickerHarness::MaxDuration)
			FNSpherePicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void RandomSimple()
	{
		FNSpherePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSpherePickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNSpherePickerHarness::MaxDuration)
			FNSpherePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Random()
	{
		FNSpherePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSpherePickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNSpherePickerHarness::MaxDuration)
			FNSpherePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void TrackedSimple()
	{
		FNSpherePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSpherePickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNSpherePickerHarness::MaxDuration)
			FNSpherePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Tracked()
	{
		FNSpherePickerParams Params;
		Params.Origin = FVector::ZeroVector;
		Params.Count = 1000;
		Params.MinimumRadius = 1.f;
		Params.MaximumRadius = 100.f;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSpherePickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNSpherePickerHarness::MaxDuration)
			FNSpherePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNSpherePickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNSpherePicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSpherePickerPerfTests::Next));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNSpherePickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNSpherePicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSpherePickerPerfTests::RandomSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNSpherePickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNSpherePicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSpherePickerPerfTests::Random));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNSpherePickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNSpherePicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSpherePickerPerfTests::TrackedSimple));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNSpherePickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNSpherePicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSpherePickerPerfTests::Tracked));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
