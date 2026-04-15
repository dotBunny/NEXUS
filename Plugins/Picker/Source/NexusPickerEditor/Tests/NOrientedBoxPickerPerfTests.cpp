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

N_TEST_PERF(FNOrientedBoxPickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumDimensions = FVector(100.f);
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
		FNOrientedBoxPicker::Next(Points, Params);
		NTestTimer.ManualStop();
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumDimensions = FVector(10.f);
	Params.MaximumDimensions = FVector(100.f);
	Params.Rotation = FRotator(0.f, 45.f, 0.f);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNOrientedBoxPickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNOrientedBoxPickerHarness::MaxDuration)
		FNOrientedBoxPicker::Next(Points, Params);
		NTestTimer.ManualStop();
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNOrientedBoxPickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNOrientedBoxPicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
