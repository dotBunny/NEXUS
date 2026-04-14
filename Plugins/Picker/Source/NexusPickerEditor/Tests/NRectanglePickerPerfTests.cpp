// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NRectanglePicker.h"
#include "NRectanglePickerParams.h"
#include "Misc/Timespan.h"

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNRectanglePicker
{
	constexpr float MaxDuration = 0.2f;
}

N_TEST_PERF(FNRectanglePickerPerfTests__NextSimple, "NEXUS::PerfTests::NPicker::FNRectanglePicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests__NextSimple, NEXUS::PerfTests::NPicker::FNRectanglePicker::MaxDuration)
		FNRectanglePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNRectanglePickerPerfTests__Next, "NEXUS::PerfTests::NPicker::FNRectanglePicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumDimensions = FVector2D(1, 1);
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests__Next, NEXUS::PerfTests::NPicker::FNRectanglePicker::MaxDuration)
		FNRectanglePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNRectanglePickerPerfTests__RandomSimple, "NEXUS::PerfTests::NPicker::FNRectanglePicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests__RandomSimple, NEXUS::PerfTests::NPicker::FNRectanglePicker::MaxDuration)
		FNRectanglePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNRectanglePickerPerfTests__Random, "NEXUS::PerfTests::NPicker::FNRectanglePicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumDimensions = FVector2D(1, 1);
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests__Random, NEXUS::PerfTests::NPicker::FNRectanglePicker::MaxDuration)
		FNRectanglePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNRectanglePickerPerfTests__TrackedSimple, "NEXUS::PerfTests::NPicker::FNRectanglePicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests__TrackedSimple, NEXUS::PerfTests::NPicker::FNRectanglePicker::MaxDuration)
		FNRectanglePicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNRectanglePickerPerfTests__Tracked, "NEXUS::PerfTests::NPicker::FNRectanglePicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumDimensions = FVector2D(1, 1);
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNRectanglePickerPerfTests__Tracked, NEXUS::PerfTests::NPicker::FNRectanglePicker::MaxDuration)
		FNRectanglePicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS