// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NSplinePicker.h"
#include "NSplinePickerParams.h"
#include "Macros/NTestMacros.h"
#include "Math/NMersenneTwister.h"

namespace NEXUS::UnitTests::NPicker::FNSplinePickerHarness
{
	constexpr float SplineLengthX = 400.f;

	// Builds an unregistered spline whose points are collinear along +X. Auto-tangents of collinear
	// points stay on the line, so every location sampled from the spline is exactly on the X axis
	// and IsPointOn's closest-point query is robust against the SplinePointTolerance check.
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

N_TEST_CRITICAL(FNSplinePickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNSplinePicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 50;
	TArray<FVector> Points;
	FNSplinePicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNSplinePickerTests_Next_PointsOnSpline, "NEXUS::UnitTests::NPicker::FNSplinePicker::Next_PointsOnSpline", N_TEST_CONTEXT_ANYWHERE)
{
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 100;
	TArray<FVector> Points;
	FNSplinePicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Next point[%d] should be on the spline"), i),
			FNSplinePicker::IsPointOn(Params.SplineComponent, Points[i]));
	}
}

N_TEST_MEDIUM(FNSplinePickerTests_Next_AppendsToExisting, "NEXUS::UnitTests::NPicker::FNSplinePicker::Next_AppendsToExisting", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that pickers append to OutLocations rather than clearing previously generated points.
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 10;
	const FVector Sentinel(-999.f, -999.f, -999.f);
	TArray<FVector> Points;
	Points.Add(Sentinel);
	FNSplinePicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should append Count points after the existing entries"), Points.Num() == 11);
	CHECK_MESSAGE(TEXT("Next should not modify pre-existing entries"), Points[0].Equals(Sentinel, 0.001f));
}

N_TEST_HIGH(FNSplinePickerTests_Random_PointsOnSpline, "NEXUS::UnitTests::NPicker::FNSplinePicker::Random_PointsOnSpline", N_TEST_CONTEXT_ANYWHERE)
{
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 100;
	TArray<FVector> Points;
	FNSplinePicker::Random(Points, Params);
	CHECK_MESSAGE(TEXT("Random should generate the requested number of points"), Points.Num() == 100);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random point[%d] should be on the spline"), i),
			FNSplinePicker::IsPointOn(Params.SplineComponent, Points[i]));
	}
}

N_TEST_HIGH(FNSplinePickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNSplinePicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 20;

	int32 SeedA = 99;
	int32 SeedB = 99;
	TArray<FVector> PointsA, PointsB;
	FNSplinePicker::Tracked(PointsA, SeedA, Params);
	FNSplinePicker::Tracked(PointsB, SeedB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked spline point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
	CHECK_MESSAGE(TEXT("Identical starting seeds should advance to identical values"), SeedA == SeedB);
}

N_TEST_MEDIUM(FNSplinePickerTests_Tracked_SeedAdvances, "NEXUS::UnitTests::NPicker::FNSplinePicker::Tracked_SeedAdvances", N_TEST_CONTEXT_ANYWHERE)
{
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 10;

	int32 Seed = 12345;
	TArray<FVector> Points;
	FNSplinePicker::Tracked(Points, Seed, Params);
	CHECK_MESSAGE(TEXT("Tracked should advance the seed so sequential calls produce new points"), Seed != 12345);
}

N_TEST_HIGH(FNSplinePickerTests_OneShot_MatchesTracked, "NEXUS::UnitTests::NPicker::FNSplinePicker::OneShot_MatchesTracked", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that OneShot produces the same sequence as Tracked for the same seed,
	// without requiring the caller to maintain seed state.
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 20;

	int32 TrackedSeed = 555;
	TArray<FVector> TrackedPoints, OneShotPoints;
	FNSplinePicker::Tracked(TrackedPoints, TrackedSeed, Params);
	FNSplinePicker::OneShot(OneShotPoints, 555, Params);
	for (int32 i = 0; i < TrackedPoints.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("OneShot point[%d] should match Tracked for the same seed"), i),
			OneShotPoints[i].Equals(TrackedPoints[i], 0.001f));
	}
}

N_TEST_HIGH(FNSplinePickerTests_Twisted_Determinism, "NEXUS::UnitTests::NPicker::FNSplinePicker::Twisted_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNSplinePickerParams Params;
	Params.SplineComponent = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	Params.Count = 20;

	FNMersenneTwister TwisterA(0xC11C1E);
	FNMersenneTwister TwisterB(0xC11C1E);
	TArray<FVector> PointsA, PointsB;
	FNSplinePicker::Twisted(PointsA, TwisterA, Params);
	FNSplinePicker::Twisted(PointsB, TwisterB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Twisted spline point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
		CHECK_MESSAGE(FString::Printf(TEXT("Twisted spline point[%d] should be on the spline"), i),
			FNSplinePicker::IsPointOn(Params.SplineComponent, PointsA[i]));
	}
}

N_TEST_HIGH(FNSplinePickerTests_IsPointOn_NullComponent, "NEXUS::UnitTests::NPicker::FNSplinePicker::IsPointOn_NullComponent", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("A null spline component should never contain a point"),
		FNSplinePicker::IsPointOn(nullptr, FVector::ZeroVector));
}

N_TEST_HIGH(FNSplinePickerTests_IsPointOn_PointsOnSpline, "NEXUS::UnitTests::NPicker::FNSplinePicker::IsPointOn_PointsOnSpline", N_TEST_CONTEXT_ANYWHERE)
{
	const USplineComponent* Spline = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	const float SplineLength = Spline->GetSplineLength();
	CHECK_MESSAGE(TEXT("The spline start should be on the spline"),
		FNSplinePicker::IsPointOn(Spline, Spline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World)));
	CHECK_MESSAGE(TEXT("A mid-length location should be on the spline"),
		FNSplinePicker::IsPointOn(Spline, Spline->GetLocationAtDistanceAlongSpline(SplineLength * 0.37f, ESplineCoordinateSpace::World)));
	CHECK_MESSAGE(TEXT("The spline end should be on the spline"),
		FNSplinePicker::IsPointOn(Spline, Spline->GetLocationAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World)));
}

N_TEST_HIGH(FNSplinePickerTests_IsPointOn_PointOffSpline, "NEXUS::UnitTests::NPicker::FNSplinePicker::IsPointOn_PointOffSpline", N_TEST_CONTEXT_ANYWHERE)
{
	const USplineComponent* Spline = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	CHECK_FALSE_MESSAGE(TEXT("A point far from the spline should not be on it"),
		FNSplinePicker::IsPointOn(Spline, FVector(200.f, 500.f, 0.f)));
}

N_TEST_HIGH(FNSplinePickerTests_IsPointsOn_MixedPoints, "NEXUS::UnitTests::NPicker::FNSplinePicker::IsPointsOn_MixedPoints", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that IsPointsOn preserves input ordering when mapping points to results.
	const USplineComponent* Spline = NEXUS::UnitTests::NPicker::FNSplinePickerHarness::CreateStraightSpline();
	const TArray<FVector> Points = {
		Spline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World),
		FVector(200.f, 500.f, 0.f),
		Spline->GetLocationAtDistanceAlongSpline(Spline->GetSplineLength() * 0.5f, ESplineCoordinateSpace::World)
	};
	const TArray<bool> Results = FNSplinePicker::IsPointsOn(Points, Spline);
	CHECK_MESSAGE(TEXT("IsPointsOn should return one result per input point"), Results.Num() == 3);
	CHECK_MESSAGE(TEXT("The on-spline start point should report true"), Results[0]);
	CHECK_FALSE_MESSAGE(TEXT("The off-spline point should report false"), Results[1]);
	CHECK_MESSAGE(TEXT("The on-spline middle point should report true"), Results[2]);
}

N_TEST_MEDIUM(FNSplinePickerTests_InvalidComponent_DefaultsToOrigin, "NEXUS::UnitTests::NPicker::FNSplinePicker::InvalidComponent_DefaultsToOrigin", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that a missing spline component fails gracefully — Count origin points and a logged error
	// rather than a crash or an empty result.
	AddExpectedMessage(TEXT("Unable to pick points on spline"), ELogVerbosity::Error);

	FNSplinePickerParams Params;
	Params.Count = 5;
	TArray<FVector> Points;
	FNSplinePicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("An invalid spline should still produce the requested number of points"), Points.Num() == 5);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Fallback point[%d] should be the origin"), i),
			Points[i].Equals(FVector::ZeroVector, 0.001f));
	}
}

#endif //WITH_TESTS
