// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NMersenneTwister.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_FloatGeneration, "NEXUS::PerfTests::NCore::FNMersenneTwister::FloatGeneration", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

	FNMersenneTwister Twister(12345ull);
	{
		N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_FloatGeneration, 10.f)
		for (int32 i = 0; i < 10000; ++i)
		{
			Twister.Float();
		}
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_IntegerRangeGeneration, "NEXUS::PerfTests::NCore::FNMersenneTwister::IntegerRangeGeneration", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

	FNMersenneTwister Twister(67890ull);
	{
		N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_IntegerRangeGeneration, 10.f)
		for (int32 i = 0; i < 10000; ++i)
		{
			Twister.IntegerRange(0, 1000);
		}
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_BulkFloatArray, "NEXUS::PerfTests::NCore::FNMersenneTwister::BulkFloatArray", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

	FNMersenneTwister Twister(11111ull);
	TArray<float> Output;
	Output.SetNum(10000);

	{
		N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_BulkFloatArray, 10.f)
		Twister.Float(Output, 10000);
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF_HIGH(FNMersenneTwisterPerfTests_BulkBoolArray, "NEXUS::PerfTests::NCore::FNMersenneTwister::BulkBoolArray", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

	FNMersenneTwister Twister(22222ull);
	TArray<bool> Output;
	Output.SetNum(10000);

	{
		N_TEST_TIMER_SCOPE(FNMersenneTwisterPerfTests_BulkBoolArray, 10.f)
		Twister.Bool(Output, 10000);
	}

	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
