// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NCirclePicker.h"
#include "NCirclePickerParams.h"
#include "Misc/Timespan.h"

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNCirclePicker
{
	constexpr float MaxDuration = 0.2f;
}

N_TEST_PERF(FNCirclePickerPerfTests_NextSimple, "NEXUS::PerfTests::NPicker::FNCirclePicker::NextSimple", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_NextSimple, NEXUS::PerfTests::NPicker::FNCirclePicker::MaxDuration)
		FNCirclePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNCirclePickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNCirclePicker::Next", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNCirclePicker::MaxDuration)
		FNCirclePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNCirclePickerPerfTests_RandomSimple, "NEXUS::PerfTests::NPicker::FNCirclePicker::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_RandomSimple, NEXUS::PerfTests::NPicker::FNCirclePicker::MaxDuration)
		FNCirclePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNCirclePickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNCirclePicker::Random", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNCirclePicker::MaxDuration)
		FNCirclePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNCirclePickerPerfTests_TrackedSimple, "NEXUS::PerfTests::NPicker::FNCirclePicker::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_TrackedSimple, NEXUS::PerfTests::NPicker::FNCirclePicker::MaxDuration)
		FNCirclePicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNCirclePickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNCirclePicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
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
		N_TEST_TIMER_SCOPE(FNCirclePickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNCirclePicker::MaxDuration)
		FNCirclePicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS