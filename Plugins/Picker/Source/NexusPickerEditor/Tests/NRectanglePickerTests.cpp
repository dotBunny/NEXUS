// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NRectanglePicker.h"
#include "NRectanglePickerParams.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNRectanglePickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MaximumDimensions = FVector2D(100.f, 100.f);
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNRectanglePickerTests_Next_PointsInsideRectangle, "NEXUS::UnitTests::NPicker::FNRectanglePicker::Next_PointsInsideRectangle", N_TEST_CONTEXT_ANYWHERE)
{
	FNRectanglePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector2D(50.f, 30.f);
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Params);
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
	TArray<FVector> Points;
	FNRectanglePicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Next rotated rectangle point[%d] should be inside"), i),
			FNRectanglePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

#endif //WITH_TESTS
