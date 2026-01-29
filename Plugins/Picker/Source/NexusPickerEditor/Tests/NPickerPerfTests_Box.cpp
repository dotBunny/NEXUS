// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

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
	Params.MaximumDimensions = FBox(FVector::ZeroVector, FVector::OneVector);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(Box_Simple_1000, 2.5f)
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
	Params.MinimumDimensions = FBox(FVector::ZeroVector, FVector::OneVector);
	Params.MaximumDimensions = FBox(FVector::ZeroVector, FVector::OneVector*100.f);
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(Box_Simple_1000, 2.5f)
		FNBoxPicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS