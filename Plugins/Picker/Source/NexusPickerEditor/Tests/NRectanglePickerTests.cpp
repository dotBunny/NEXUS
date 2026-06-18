// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NRectanglePicker.h"
#include "NRectanglePickerParams.h"
#include "Macros/NTestMacros.h"
#include "Math/NMersenneTwister.h"

N_TEST_CRITICAL(FNRectanglePickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MaximumDimensions = FVector2D(100.f, 100.f);
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Twister, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNRectanglePickerTests_Next_PointsInsideRectangle, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_PointsInsideRectangle", N_TEST_CONTEXT_ANYWHERE)
{
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector2D(50.f, 30.f);
	Params.Rotation = FRotator::ZeroRotator;
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Twister, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the rectangle"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Center, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Center", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Origin should be inside rectangle centered at origin"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, FVector2D(10.f, 10.f), FRotator::ZeroRotator, FVector::ZeroVector));
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Outside, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Outside", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Distant point should be outside rectangle"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, FVector2D(10.f, 10.f), FRotator::ZeroRotator, FVector(100.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Rotated, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Rotated", N_TEST_CONTEXT_ANYWHERE)
{
	// A 90-degree yaw rotation swaps local X/Y axes
	const FRotator Rotation(0.f, 90.f, 0.f);
	const FVector2D Dims(20.f, 4.f);
	// Without rotation, (9, 0, 0) is inside 20x4. With 90-degree yaw, local X becomes world Y.
	// Point (0, 9, 0) should be inside the rotated rectangle.
	CHECK_MESSAGE(TEXT("Point along rotated axis should be inside"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, Dims, Rotation, FVector(0.f, 9.f, 0.f)));
	// Point (9, 0, 0) should now be outside (the narrow dimension is only 4 wide)
	CHECK_FALSE_MESSAGE(TEXT("Point outside rotated narrow dimension should be outside"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, Dims, Rotation, FVector(9.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNRectanglePickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 20;
	Params.MaximumDimensions = FVector2D(100.f, 100.f);

	int32 SeedA = 55;
	int32 SeedB = 55;
	TArray<FVector> PointsA, PointsB;
	FNRectanglePicker::Tracked(PointsA, SeedA, Params);
	FNRectanglePicker::Tracked(PointsB, SeedB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked rectangle point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
}

N_TEST_HIGH(FNRectanglePickerTests_Random_PointsInsideRectangle, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Random_PointsInsideRectangle", N_TEST_CONTEXT_ANYWHERE)
{
	FNRectanglePickerParams Params;
	Params.Origin = FVector(10.f, 20.f, 0.f);
	Params.Count = 100;
	Params.MaximumDimensions = FVector2D(40.f, 60.f);
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNRectanglePicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random rectangle point[%d] should be inside"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNRectanglePickerTests_Random_PointsInsideRotatedRectangle, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Random_PointsInsideRotatedRectangle", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies Random respects non-axis-aligned rotation. Without a rotated case, a regression
	// where the implementation ignored Rotation in the random path would slip through.
	FNRectanglePickerParams Params;
	Params.Origin = FVector(5.f, -3.f, 0.f);
	Params.Count = 200;
	Params.MaximumDimensions = FVector2D(30.f, 10.f);
	Params.Rotation = FRotator(0.f, 35.f, 0.f);
	TArray<FVector> Points;
	FNRectanglePicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random rotated rectangle point[%d] should be inside"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNRectanglePickerTests_Next_PointsInsideRotatedRectangle, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_PointsInsideRotatedRectangle", N_TEST_CONTEXT_ANYWHERE)
{
	// Mirror coverage of Next under non-trivial rotation.
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector2D(30.f, 10.f);
	Params.Rotation = FRotator(0.f, 60.f, 0.f);
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Twister, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Next rotated rectangle point[%d] should be inside"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_CRITICAL(FNRectanglePickerTests_Next_DegenerateHollow, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_DegenerateHollow", N_TEST_CONTEXT_ANYWHERE)
{
	// Regression: MinimumDimensions == MaximumDimensions leaves zero annulus area, so GetValidRanges()
	// previously returned an empty array that the picker indexed out-of-bounds (a hard crash, not an
	// assertion failure). It must now fall back to the full rectangle and still return Count points.
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 16;
	Params.MinimumDimensions = FVector2D(100.f, 100.f);
	Params.MaximumDimensions = FVector2D(100.f, 100.f);
	// The degenerate-hollow fallback in GetValidRanges() logs a warning; declare it expected so this test
	// stays clean (and green under stricter automation configs that elevate log warnings to failures).
	this->AddExpectedError(TEXT("matches the MaximumDimensions on both axes"), EAutomationExpectedErrorFlags::Contains, 1);
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Twister, Params); // must not crash
	CHECK_MESSAGE(TEXT("Degenerate hollow rectangle should still return Count points"), Points.Num() == 16);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Degenerate hollow point[%d] should be inside the rectangle"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNRectanglePickerTests_Next_HollowPointsInsideOuter, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_HollowPointsInsideOuter", N_TEST_CONTEXT_ANYWHERE)
{
	// Exercises the hollow (annulus) path the existing tests never touched: MinimumDimensions set and
	// strictly inside MaximumDimensions. Every generated point must lie within the outer rectangle.
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MinimumDimensions = FVector2D(40.f, 40.f);
	Params.MaximumDimensions = FVector2D(100.f, 100.f);
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Twister, Params);
	CHECK_MESSAGE(TEXT("Hollow rectangle should return Count points"), Points.Num() == 200);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Hollow point[%d] should be inside the outer rectangle"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Band_InnerBoundary, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Band_InnerBoundary", N_TEST_CONTEXT_ANYWHERE)
{
	// A point exactly on the inner edge is part of the closed band.
	CHECK_MESSAGE(TEXT("Point on the inner edge should be inside the band"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, FVector2D(10.f, 10.f), FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector(5.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Band_OuterBoundary, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Band_OuterBoundary", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Point on the outer edge should be inside the band"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, FVector2D(10.f, 10.f), FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector(10.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Band_Hole, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Band_Hole", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Point strictly inside the inner rectangle should be excluded"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, FVector2D(10.f, 10.f), FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector(2.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInsideOrOn_Band_NoHoleIncludesCenter, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInsideOrOn_Band_NoHoleIncludesCenter", N_TEST_CONTEXT_ANYWHERE)
{
	// Zero inner dimensions leave no hole, so the center is part of the band.
	CHECK_MESSAGE(TEXT("Center should be inside the band when MinimumDimensions is zero"),
		FNRectanglePicker::IsPointInsideOrOn(FVector::ZeroVector, FVector2D::ZeroVector, FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector::ZeroVector));
}

N_TEST_HIGH(FNRectanglePickerTests_IsPointInside_Strict, "NEXUS::UnitTests::NPicker::FNRectanglePicker::IsPointInside_Strict", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Point strictly inside should be inside"),
		FNRectanglePicker::IsPointInside(FVector::ZeroVector, FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector(5.f, 0.f, 0.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point on the edge should not be strictly inside"),
		FNRectanglePicker::IsPointInside(FVector::ZeroVector, FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector(10.f, 0.f, 0.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point outside should not be strictly inside"),
		FNRectanglePicker::IsPointInside(FVector::ZeroVector, FVector2D(20.f, 20.f), FRotator::ZeroRotator, FVector(20.f, 0.f, 0.f)));
}

#endif //WITH_TESTS
