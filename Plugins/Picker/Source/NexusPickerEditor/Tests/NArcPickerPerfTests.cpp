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

N_TEST_PERF(FNArcPickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNArcPicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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
		N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
		FNArcPicker::Next(Points, Params);
		NTestTimer.ManualStop();
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNArcPickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNArcPicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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
		N_TEST_TIMER_SCOPE(FNArcPickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNArcPickerHarness::MaxDuration)
		FNArcPicker::Next(Points, Params);
		NTestTimer.ManualStop();
	}

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNArcPickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNArcPicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNArcPickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNArcPicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNArcPickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNArcPicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

N_TEST_PERF(FNArcPickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNArcPicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();

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

	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS
