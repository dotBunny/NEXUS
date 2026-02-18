// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NSpherePicker.h"
#include "NSpherePickerParams.h"
#include "Misc/Timespan.h"

#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PickerTests::Sphere
{
	constexpr float MaxDuration = 0.2f;
}

N_TEST_PERF(FNPickerPerfTests_Sphere_NextSimple, "NEXUS::PerfTests::NPicker::Sphere::NextSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Sphere_NextSimple, NEXUS::PickerTests::Sphere::MaxDuration)
		FNSpherePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Sphere_Next, "NEXUS::PerfTests::NPicker::Sphere::Next", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Sphere_Next, NEXUS::PickerTests::Sphere::MaxDuration)
		FNSpherePicker::Next(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Sphere_RandomSimple, "NEXUS::PerfTests::NPicker::Sphere::RandomSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Sphere_RandomSimple, NEXUS::PickerTests::Sphere::MaxDuration)
		FNSpherePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Sphere_Random, "NEXUS::PerfTests::NPicker::Sphere::Random", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Sphere_Random, NEXUS::PickerTests::Sphere::MaxDuration)
		FNSpherePicker::Random(Points, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Sphere_TrackedSimple, "NEXUS::PerfTests::NPicker::Sphere::TrackedSimple", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Sphere_TrackedSimple, NEXUS::PickerTests::Sphere::MaxDuration)
		FNSpherePicker::Tracked(Points, Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}
N_TEST_PERF(FNPickerPerfTests_Sphere_Tracked, "NEXUS::PerfTests::NPicker::Sphere::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	FNTestUtils::PrePerformanceTest();
	
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 1000;
	Params.MinimumRadius = 1.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	int32 Seed = 123456789;
	// TEST
	{
		N_TEST_TIMER_SCOPE(FNPickerPerfTests_Sphere_Tracked, NEXUS::PickerTests::Sphere::MaxDuration)
		FNSpherePicker::Tracked(Points,Seed, Params);
		
		// Explicitly stop the timer
		NTestTimer.ManualStop();
	}
	
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS