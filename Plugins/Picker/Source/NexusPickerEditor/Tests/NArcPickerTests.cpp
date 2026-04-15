// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArcPicker.h"
#include "NArcPickerParams.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNArcPickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNArcPicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNArcPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.Rotation = FRotator::ZeroRotator;
	Params.Degrees = 90.f;
	Params.MinimumDistance = 0.f;
	Params.MaximumDistance = 100.f;
	TArray<FVector> Points;
	FNArcPicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNArcPickerTests_Next_PointsInsideArc, "NEXUS::UnitTests::NPicker::FNArcPicker::Next_PointsInsideArc", N_TEST_CONTEXT_ANYWHERE)
{
	FNArcPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.Rotation = FRotator(0.f, 0.f, 0.f);
	Params.Degrees = 180.f;
	Params.MinimumDistance = 0.f;
	Params.MaximumDistance = 50.f;
	TArray<FVector> Points;
	FNArcPicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the arc"), i),
			FNArcPicker::IsPointInsideOrOn(Params.Origin, Params.Rotation, Params.Degrees, Params.MinimumDistance, Params.MaximumDistance, Points[i]));
	}
}

N_TEST_HIGH(FNArcPickerTests_IsPointInsideOrOn_WithinArc, "NEXUS::UnitTests::NPicker::FNArcPicker::IsPointInsideOrOn_WithinArc", N_TEST_CONTEXT_ANYWHERE)
{
	// Forward-facing arc (default rotation faces +X), 90 degrees wide, distance 5..50
	const FVector Origin = FVector::ZeroVector;
	const FRotator Rotation = FRotator::ZeroRotator;
	constexpr float Degrees = 90.f;
	constexpr float MinDist = 5.f;
	constexpr float MaxDist = 50.f;
	// Point directly forward at distance 25 should be inside
	CHECK_MESSAGE(TEXT("Point directly forward should be inside arc"),
		FNArcPicker::IsPointInsideOrOn(Origin, Rotation, Degrees, MinDist, MaxDist, FVector(25.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNArcPickerTests_IsPointInsideOrOn_OutsideDistance, "NEXUS::UnitTests::NPicker::FNArcPicker::IsPointInsideOrOn_OutsideDistance", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Point beyond max distance should be outside"),
		FNArcPicker::IsPointInsideOrOn(FVector::ZeroVector, FRotator::ZeroRotator, 90.f, 5.f, 50.f, FVector(100.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNArcPickerTests_IsPointInsideOrOn_TooClose, "NEXUS::UnitTests::NPicker::FNArcPicker::IsPointInsideOrOn_TooClose", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Point inside minimum distance should be outside"),
		FNArcPicker::IsPointInsideOrOn(FVector::ZeroVector, FRotator::ZeroRotator, 90.f, 10.f, 50.f, FVector(2.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNArcPickerTests_IsPointInsideOrOn_OutsideAngle, "NEXUS::UnitTests::NPicker::FNArcPicker::IsPointInsideOrOn_OutsideAngle", N_TEST_CONTEXT_ANYWHERE)
{
	// 90 degree arc facing +X should not include point directly behind (-X)
	CHECK_FALSE_MESSAGE(TEXT("Point behind the arc direction should be outside"),
		FNArcPicker::IsPointInsideOrOn(FVector::ZeroVector, FRotator::ZeroRotator, 90.f, 0.f, 50.f, FVector(-25.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNArcPickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNArcPicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNArcPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 20;
	Params.Rotation = FRotator::ZeroRotator;
	Params.Degrees = 120.f;
	Params.MinimumDistance = 0.f;
	Params.MaximumDistance = 100.f;

	int32 SeedA = 314;
	int32 SeedB = 314;
	TArray<FVector> PointsA, PointsB;
	FNArcPicker::Tracked(PointsA, SeedA, Params);
	FNArcPicker::Tracked(PointsB, SeedB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked arc point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
}

N_TEST_HIGH(FNArcPickerTests_Random_PointsInsideArc, "NEXUS::UnitTests::NPicker::FNArcPicker::Random_PointsInsideArc", N_TEST_CONTEXT_ANYWHERE)
{
	FNArcPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 100;
	Params.Rotation = FRotator::ZeroRotator;
	Params.Degrees = 360.f;
	Params.MinimumDistance = 0.f;
	Params.MaximumDistance = 50.f;
	TArray<FVector> Points;
	FNArcPicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random arc point[%d] should be inside full arc"), i),
			FNArcPicker::IsPointInsideOrOn(Params.Origin, Params.Rotation, Params.Degrees, Params.MinimumDistance, Params.MaximumDistance, Points[i]));
	}
}

#endif //WITH_TESTS
