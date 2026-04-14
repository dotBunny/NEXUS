// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NBoxPicker.h"
#include "NBoxPickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNBoxPickerHarness
{
	constexpr float MaxDuration = 0.2f;
}

N_TEST_PERF(FNBoxPickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNBoxPicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNBoxPickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNBoxPickerHarness::MaxDuration)
		FNBoxPicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNBoxPickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNBoxPicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumBox = FBox(FVector::OneVector, FVector::OneVector);
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector*100.f);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNBoxPickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNBoxPickerHarness::MaxDuration)
		FNBoxPicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNBoxPickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNBoxPicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNBoxPickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNBoxPickerHarness::MaxDuration)
		FNBoxPicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNBoxPickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNBoxPicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumBox = FBox(FVector::OneVector, FVector::OneVector);
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector*100.f);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNBoxPickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNBoxPickerHarness::MaxDuration)
		FNBoxPicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNBoxPickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNBoxPicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector);
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNBoxPickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNBoxPickerHarness::MaxDuration)
		FNBoxPicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNBoxPickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNBoxPicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumBox = FBox(FVector::OneVector, FVector::OneVector);
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector*100.f);
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNBoxPickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNBoxPickerHarness::MaxDuration)
		FNBoxPicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS