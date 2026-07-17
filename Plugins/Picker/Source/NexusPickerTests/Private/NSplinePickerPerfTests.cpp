// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NSplinePicker.h"
#include "NSplinePickerParams.h"
#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NPicker::FNSplinePickerHarness
{
	constexpr float MaxDuration = 0.2f;
	constexpr float SplineLengthX = 400.f;

	// Builds an unregistered spline whose points are collinear along +X; spline setup happens
	// outside the timed region so only the picking cost is measured.
	inline USplineComponent* CreateStraightSpline()
	{
		USplineComponent* Spline = NewObject<USplineComponent>(GetTransientPackage());
		Spline->ClearSplinePoints(false);
		Spline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::World, false);
		Spline->AddSplinePoint(FVector(SplineLengthX * 0.5f, 0.f, 0.f), ESplineCoordinateSpace::World, false);
		Spline->AddSplinePoint(FVector(SplineLengthX, 0.f, 0.f), ESplineCoordinateSpace::World, false);
		Spline->UpdateSpline();
		return Spline;
	}
}

class FNSplinePickerPerfTests
{
public:
	static void Next()
	{
		FNSplinePickerParams Params;
		Params.SplineComponent = NEXUS::PerfTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
		Params.Count = 1000;
		TArray<FVector> Points;
		FNMersenneTwister Twister(123456789);
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSplinePickerPerfTests_Next, NEXUS::PerfTests::NPicker::FNSplinePickerHarness::MaxDuration)
			FNSplinePicker::Next(Points, Twister, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Random()
	{
		FNSplinePickerParams Params;
		Params.SplineComponent = NEXUS::PerfTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
		Params.Count = 1000;
		TArray<FVector> Points;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSplinePickerPerfTests_Random, NEXUS::PerfTests::NPicker::FNSplinePickerHarness::MaxDuration)
			FNSplinePicker::Random(Points, Params);
			NTestTimer.ManualStop();
		}
	}

	static void Tracked()
	{
		FNSplinePickerParams Params;
		Params.SplineComponent = NEXUS::PerfTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
		Params.Count = 1000;
		TArray<FVector> Points;
		int32 Seed = 123456789;
		// TEST
		{
			N_TEST_TIMER_SCOPE(FNSplinePickerPerfTests_Tracked, NEXUS::PerfTests::NPicker::FNSplinePickerHarness::MaxDuration)
			FNSplinePicker::Tracked(Points, Seed, Params);
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNSplinePickerPerfTests_Next, "NEXUS::PerfTests::NPicker::FNSplinePicker::Next", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSplinePickerPerfTests::Next));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNSplinePickerPerfTests_Random, "NEXUS::PerfTests::NPicker::FNSplinePicker::Random", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSplinePickerPerfTests::Random));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNSplinePickerPerfTests_Tracked, "NEXUS::PerfTests::NPicker::FNSplinePicker::Tracked", N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNSplinePickerPerfTests::Tracked));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
