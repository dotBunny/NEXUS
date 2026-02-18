// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

namespace NEXUS::PickerTests::Rectangle
{
	constexpr float MaxDuration = 0.2f;
}
#include "NRectanglePicker.h"
#include "NRectanglePickerParams.h"
#include "Misc/Timespan.h"

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNPickerPerfTests_Rectangle_NextSimple, "NEXUS::PerfTests::NPicker::Rectangle::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Rectangle_NextSimple, NEXUS::PickerTests::Rectangle::MaxDuration)
		FNRectanglePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Rectangle_Next, "NEXUS::PerfTests::NPicker::Rectangle::Next", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Rectangle_Next, NEXUS::PickerTests::Rectangle::MaxDuration)
		FNRectanglePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Rectangle_RandomSimple, "NEXUS::PerfTests::NPicker::Rectangle::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MaximumDimensions = FVector2D(100, 100);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Rectangle_RandomSimple, NEXUS::PickerTests::Rectangle::MaxDuration)
		FNRectanglePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Rectangle_Random, "NEXUS::PerfTests::NPicker::Rectangle::Random", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Rectangle_Random, NEXUS::PickerTests::Rectangle::MaxDuration)
		FNRectanglePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Rectangle_TrackedSimple, "NEXUS::PerfTests::NPicker::Rectangle::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Rectangle_TrackedSimple, NEXUS::PickerTests::Rectangle::MaxDuration)
		FNRectanglePicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Rectangle_Tracked, "NEXUS::PerfTests::NPicker::Rectangle::Tracked", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Rectangle_Tracked, NEXUS::PickerTests::Rectangle::MaxDuration)
		FNRectanglePicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS