// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NOrientedBoxPicker.h"
#include "NOrientedBoxPickerParams.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNOrientedBoxPickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MaximumDimensions = FVector(100.f);
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNOrientedBoxPickerTests_Next_PointsInsideBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Next_PointsInsideBox", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector(50.f, 30.f, 20.f);
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the oriented box"), i),
			FNOrientedBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Center, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Center", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Origin should be inside box centered at origin"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, FVector(10.f), FRotator::ZeroRotator, FVector::ZeroVector));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Outside, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Outside", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Distant point should be outside"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, FVector(10.f), FRotator::ZeroRotator, FVector(100.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Rotated, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Rotated", N_TEST_CONTEXT_ANYWHERE)
{
	// 45-degree yaw rotation on a long thin box
	const FVector Dims(20.f, 2.f, 2.f);
	const FRotator Rotation(0.f, 45.f, 0.f);
	// Point at (7, 7, 0) should be inside a 20-unit long box rotated 45 degrees
	CHECK_MESSAGE(TEXT("Point along rotated diagonal should be inside"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, Dims, Rotation, FVector(7.f, 7.f, 0.f)));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 20;
	Params.MaximumDimensions = FVector(100.f);
	Params.Rotation = FRotator::ZeroRotator;

	int32 SeedA = 271;
	int32 SeedB = 271;
	TArray<FVector> PointsA, PointsB;
	FNOrientedBoxPicker::Tracked(PointsA, SeedA, Params);
	FNOrientedBoxPicker::Tracked(PointsB, SeedB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked oriented box point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
}

N_TEST_HIGH(FNOrientedBoxPickerTests_Random_PointsInsideBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Random_PointsInsideBox", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector(10.f, 20.f, 30.f);
	Params.Count = 100;
	Params.MaximumDimensions = FVector(40.f, 60.f, 20.f);
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNOrientedBoxPicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random oriented box point[%d] should be inside"), i),
			FNOrientedBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNOrientedBoxPickerTests_Tracked_SeedMutates, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Tracked_SeedMutates", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 5;
	Params.MaximumDimensions = FVector(10.f);
	Params.Rotation = FRotator::ZeroRotator;

	int32 Seed = 12345;
	const int32 OriginalSeed = Seed;
	TArray<FVector> Points;
	FNOrientedBoxPicker::Tracked(Points, Seed, Params);
	CHECK_MESSAGE(TEXT("Tracked should mutate the seed"), Seed != OriginalSeed);
}

N_TEST_HIGH(FNOrientedBoxPickerTests_Random_PointsInsideRotatedBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Random_PointsInsideRotatedBox", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies Random respects rotation. The companion Random_PointsInsideBox uses zero rotation
	// and would not catch a bug where the implementation dropped the Rotation parameter on the
	// random path.
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector(5.f, 10.f, -2.f);
	Params.Count = 200;
	Params.MaximumDimensions = FVector(40.f, 8.f, 8.f);
	Params.Rotation = FRotator(15.f, 50.f, 25.f);
	TArray<FVector> Points;
	FNOrientedBoxPicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random rotated oriented box point[%d] should be inside"), i),
			FNOrientedBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNOrientedBoxPickerTests_Next_PointsInsideRotatedBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Next_PointsInsideRotatedBox", N_TEST_CONTEXT_ANYWHERE)
{
	// Mirror Random coverage on the deterministic Next path.
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector(50.f, 10.f, 5.f);
	Params.Rotation = FRotator(0.f, 45.f, 30.f);
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Next rotated oriented box point[%d] should be inside"), i),
			FNOrientedBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

#endif //WITH_TESTS
