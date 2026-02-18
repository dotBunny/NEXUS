// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#define N_CIRCLE_PICKER_MAX_TEST_TIMER 0.2f

#include "NCirclePicker.h"
#include "NCirclePickerParams.h"
#include "Misc/Timespan.h"

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNPickerPerfTests_Circle_NextSimple, "NEXUS::PerfTests::NPicker::Circle::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Circle_NextSimple, N_CIRCLE_PICKER_MAX_TEST_TIMER)
		FNCirclePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Circle_Next, "NEXUS::PerfTests::NPicker::Circle::Next", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	Params.Rotation = FRotator(10.4);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Circle_Next, N_CIRCLE_PICKER_MAX_TEST_TIMER)
		FNCirclePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Circle_RandomSimple, "NEXUS::PerfTests::NPicker::Circle::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Circle_RandomSimple, N_CIRCLE_PICKER_MAX_TEST_TIMER)
		FNCirclePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Circle_Random, "NEXUS::PerfTests::NPicker::Circle::Random", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	Params.Rotation = FRotator(10.4);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Circle_Random, N_CIRCLE_PICKER_MAX_TEST_TIMER)
		FNCirclePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Circle_TrackedSimple, "NEXUS::PerfTests::NPicker::Circle::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Circle_TrackedSimple, N_CIRCLE_PICKER_MAX_TEST_TIMER)
		FNCirclePicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Circle_Tracked, "NEXUS::PerfTests::NPicker::Circle::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
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
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Circle_Tracked, N_CIRCLE_PICKER_MAX_TEST_TIMER)
		FNCirclePicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS