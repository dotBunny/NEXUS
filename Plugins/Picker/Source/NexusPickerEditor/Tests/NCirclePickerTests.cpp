// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NCirclePicker.h"
#include "NCirclePickerParams.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNCirclePickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNCirclePicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	FNCirclePicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNCirclePickerTests_Next_PointsInsideAnnulus, "NEXUS::UnitTests::NPicker::FNCirclePicker::Next_PointsInsideAnnulus", N_TEST_CONTEXT_ANYWHERE)
{
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 50.f;
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNCirclePicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the circle"), i),
			FNCirclePicker::IsPointInsideOrOn(Params.Origin, Params.MinimumRadius, Params.MaximumRadius, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNCirclePickerTests_IsPointInsideOrOn_Center, "NEXUS::UnitTests::NPicker::FNCirclePicker::IsPointInsideOrOn_Center", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Center should be inside circle with zero inner radius"),
		FNCirclePicker::IsPointInsideOrOn(FVector::ZeroVector, 0.f, 10.f, FRotator::ZeroRotator, FVector::ZeroVector));
}

N_TEST_HIGH(FNCirclePickerTests_IsPointInsideOrOn_OutsideRadius, "NEXUS::UnitTests::NPicker::FNCirclePicker::IsPointInsideOrOn_OutsideRadius", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Point beyond max radius should be outside"),
		FNCirclePicker::IsPointInsideOrOn(FVector::ZeroVector, 0.f, 10.f, FRotator::ZeroRotator, FVector(100.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNCirclePickerTests_IsPointInsideOrOn_InsideHole, "NEXUS::UnitTests::NPicker::FNCirclePicker::IsPointInsideOrOn_InsideHole", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that points inside the inner radius hole are rejected.
	CHECK_FALSE_MESSAGE(TEXT("Center should be outside annulus with inner radius > 0"),
		FNCirclePicker::IsPointInsideOrOn(FVector::ZeroVector, 5.f, 10.f, FRotator::ZeroRotator, FVector::ZeroVector));
}

N_TEST_HIGH(FNCirclePickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNCirclePicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 20;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 50.f;

	int32 SeedA = 99;
	int32 SeedB = 99;
	TArray<FVector> PointsA, PointsB;
	FNCirclePicker::Tracked(PointsA, SeedA, Params);
	FNCirclePicker::Tracked(PointsB, SeedB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked circle point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
}

N_TEST_HIGH(FNCirclePickerTests_Random_PointsInsideAnnulus, "NEXUS::UnitTests::NPicker::FNCirclePicker::Random_PointsInsideAnnulus", N_TEST_CONTEXT_ANYWHERE)
{
	FNCirclePickerParams Params;
	Params.Origin = FVector(10.f, 20.f, 0.f);
	Params.Count = 100;
	Params.MinimumRadius = 5.f;
	Params.MaximumRadius = 30.f;
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNCirclePicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random circle point[%d] should be inside annulus"), i),
			FNCirclePicker::IsPointInsideOrOn(Params.Origin, Params.MinimumRadius, Params.MaximumRadius, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNCirclePickerTests_Next_PointsInsideAnnulusInnerRadius, "NEXUS::UnitTests::NPicker::FNCirclePicker::Next_PointsInsideAnnulusInnerRadius", N_TEST_CONTEXT_ANYWHERE)
{
	// The default Next_PointsInsideAnnulus test uses MinimumRadius = 0 so it cannot detect a
	// regression that ignores the inner radius. This case asserts that no Next point falls in the hole.
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MinimumRadius = 8.f;
	Params.MaximumRadius = 25.f;
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNCirclePicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Next annulus point[%d] should respect the non-zero inner radius"), i),
			FNCirclePicker::IsPointInsideOrOn(Params.Origin, Params.MinimumRadius, Params.MaximumRadius, Params.Rotation, Points[i]));
	}
}

#endif //WITH_TESTS
