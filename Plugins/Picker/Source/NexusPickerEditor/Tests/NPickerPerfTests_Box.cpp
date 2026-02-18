// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

namespace NEXUS::PickerTests::Box
{
	constexpr float MaxDuration = 0.2f;
}

#include "NBoxPicker.h"
#include "NBoxPickerParams.h"
#include "Misc/Timespan.h"

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNPickerPerfTests_Box_NextSimple, "NEXUS::PerfTests::NPicker::Box::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Box_NextSimple, NEXUS::PickerTests::Box::MaxDuration)
		FNBoxPicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Box_Next, "NEXUS::PerfTests::NPicker::Box::Next", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Box_Next, NEXUS::PickerTests::Box::MaxDuration)
		FNBoxPicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Box_RandomSimple, "NEXUS::PerfTests::NPicker::Box::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumBox = FBox(FVector::ZeroVector, FVector::OneVector);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Box_RandomSimple, NEXUS::PickerTests::Box::MaxDuration)
		FNBoxPicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Box_Random, "NEXUS::PerfTests::NPicker::Box::Random", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Box_Random, NEXUS::PickerTests::Box::MaxDuration)
		FNBoxPicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Box_TrackedSimple, "NEXUS::PerfTests::NPicker::Box::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Box_TrackedSimple, NEXUS::PickerTests::Box::MaxDuration)
		FNBoxPicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Box_Tracked, "NEXUS::PerfTests::NPicker::Box::Tracked", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Box_Tracked, NEXUS::PickerTests::Box::MaxDuration)
		FNBoxPicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS