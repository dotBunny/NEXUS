// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NMersenneTwister.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NCore::FNMersenneTwisterHarness
{
	constexpr int32 SampleCount = 10000;
	constexpr float MaxDuration = 10.f;
}

class FNMersenneTwisterPerfTests
{
public:
	static void FloatGeneration()
	{
		FNMersenneTwister Twister(12345ull);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_FloatGeneration,
				NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::MaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::SampleCount; ++i)
			{
				Twister.Float();
			}
			NTestTimer.ManualStop();
		}
	}

	static void IntegerRangeGeneration()
	{
		FNMersenneTwister Twister(67890ull);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_IntegerRangeGeneration,
				NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::MaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::SampleCount; ++i)
			{
				Twister.IntegerRange(0, 1000);
			}
			NTestTimer.ManualStop();
		}
	}

	static void BulkFloatArray()
	{
		FNMersenneTwister Twister(11111ull);
		TArray<float> Output;
		Output.SetNum(NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::SampleCount);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_BulkFloatArray,
				NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::MaxDuration)
			Twister.Float(Output, NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::SampleCount);
			NTestTimer.ManualStop();
		}
	}

	static void BulkBoolArray()
	{
		FNMersenneTwister Twister(22222ull);
		TArray<bool> Output;
		Output.SetNum(NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::SampleCount);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_BulkBoolArray,
				NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::MaxDuration)
			Twister.Bool(Output, NEXUS::PerfTests::NCore::FNMersenneTwisterHarness::SampleCount);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_FloatGeneration,
	"NEXUS::PerfTests::NCore::FNMersenneTwister::FloatGeneration",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNMersenneTwisterPerfTests::FloatGeneration));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_IntegerRangeGeneration,
	"NEXUS::PerfTests::NCore::FNMersenneTwister::IntegerRangeGeneration",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNMersenneTwisterPerfTests::IntegerRangeGeneration));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_BulkFloatArray,
	"NEXUS::PerfTests::NCore::FNMersenneTwister::BulkFloatArray",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNMersenneTwisterPerfTests::BulkFloatArray));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_BulkBoolArray,
	"NEXUS::PerfTests::NCore::FNMersenneTwister::BulkBoolArray",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNMersenneTwisterPerfTests::BulkBoolArray));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
