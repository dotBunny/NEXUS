// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NOrientedBoxPicker.h"
#include "NOrientedBoxPickerParams.h"
#include "Macros/NTestMacros.h"
#include "Math/NMersenneTwister.h"
#include "Math/OrientedBox.h"

N_TEST_CRITICAL(FNOrientedBoxPickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MaximumDimensions = FVector(100.f);
	Params.Rotation = FRotator::ZeroRotator;
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Twister, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNOrientedBoxPickerTests_Next_PointsInsideBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Next_PointsInsideBox", N_TEST_CONTEXT_ANYWHERE)
{
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector(50.f, 30.f, 20.f);
	Params.Rotation = FRotator::ZeroRotator;
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Twister, Params);
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

N_TEST_HIGH(FNOrientedBoxPickerParamsTests_InitializeFrom_MatchesSource, "NEXUS::UnitTests::NPicker::FNOrientedBoxPickerParams::InitializeFrom_MatchesSource", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that InitializeFrom adopts the source box's center, rotation, and local dimensions
	// rather than the world-space AABB of the rotated corners.
	const FRotator SourceRotation(10.f, 45.f, 20.f);
	const FQuat SourceQuat = SourceRotation.Quaternion();
	FOrientedBox OrientedBox;
	OrientedBox.Center = FVector(25.f, -10.f, 5.f);
	OrientedBox.AxisX = SourceQuat.GetAxisX();
	OrientedBox.AxisY = SourceQuat.GetAxisY();
	OrientedBox.AxisZ = SourceQuat.GetAxisZ();
	OrientedBox.ExtentX = 20.f;
	OrientedBox.ExtentY = 10.f;
	OrientedBox.ExtentZ = 5.f;

	FNOrientedBoxPickerParams Params;
	Params.InitializeFrom(OrientedBox);

	CHECK_MESSAGE(TEXT("Origin should match the source box center"),
		Params.Origin.Equals(OrientedBox.Center, 0.001f));
	CHECK_MESSAGE(TEXT("MaximumDimensions should be twice the source extents"),
		Params.MaximumDimensions.Equals(FVector(40.f, 20.f, 10.f), 0.001f));
	CHECK_MESSAGE(TEXT("Rotation should match the source box orientation"),
		Params.Rotation.Equals(SourceRotation, 0.001f));
}

N_TEST_HIGH(FNOrientedBoxPickerParamsTests_InitializeFrom_PointsInsideSourceBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPickerParams::InitializeFrom_PointsInsideSourceBox", N_TEST_CONTEXT_ANYWHERE)
{
	// A yawed cube's world-space AABB is sqrt(2) wider in X and Y; dimensions derived from that
	// AABB instead of the box extents would routinely place points outside the source box.
	const FQuat SourceQuat = FRotator(0.f, 45.f, 0.f).Quaternion();
	FOrientedBox OrientedBox;
	OrientedBox.Center = FVector(10.f, 20.f, 30.f);
	OrientedBox.AxisX = SourceQuat.GetAxisX();
	OrientedBox.AxisY = SourceQuat.GetAxisY();
	OrientedBox.AxisZ = SourceQuat.GetAxisZ();
	OrientedBox.ExtentX = 25.f;
	OrientedBox.ExtentY = 25.f;
	OrientedBox.ExtentZ = 25.f;

	FNOrientedBoxPickerParams Params;
	Params.InitializeFrom(OrientedBox);
	Params.Count = 200;

	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Twister, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		const FVector Local = Points[i] - OrientedBox.Center;
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the source oriented box"), i),
			FMath::Abs(Local | OrientedBox.AxisX) <= OrientedBox.ExtentX + 0.001f &&
			FMath::Abs(Local | OrientedBox.AxisY) <= OrientedBox.ExtentY + 0.001f &&
			FMath::Abs(Local | OrientedBox.AxisZ) <= OrientedBox.ExtentZ + 0.001f);
	}
}

N_TEST_HIGH(FNOrientedBoxPickerTests_Next_PointsInsideRotatedBox, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::Next_PointsInsideRotatedBox", N_TEST_CONTEXT_ANYWHERE)
{
	// Mirror Random coverage on the seeded Next path.
	FNOrientedBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MaximumDimensions = FVector(50.f, 10.f, 5.f);
	Params.Rotation = FRotator(0.f, 45.f, 30.f);
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNOrientedBoxPicker::Next(Points, Twister, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Next rotated oriented box point[%d] should be inside"), i),
			FNOrientedBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumDimensions, Params.Rotation, Points[i]));
	}
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Shell_InnerBoundary, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Shell_InnerBoundary", N_TEST_CONTEXT_ANYWHERE)
{
	// A point exactly on the inner surface is part of the closed shell.
	CHECK_MESSAGE(TEXT("Point on the inner surface should be inside the shell"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, FVector(10.f), FVector(20.f), FRotator::ZeroRotator, FVector(5.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Shell_OuterBoundary, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Shell_OuterBoundary", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Point on the outer surface should be inside the shell"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, FVector(10.f), FVector(20.f), FRotator::ZeroRotator, FVector(10.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Shell_Hole, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Shell_Hole", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Point strictly inside the inner box should be excluded"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, FVector(10.f), FVector(20.f), FRotator::ZeroRotator, FVector(2.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInsideOrOn_Shell_NoHoleIncludesCenter, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInsideOrOn_Shell_NoHoleIncludesCenter", N_TEST_CONTEXT_ANYWHERE)
{
	// Zero inner dimensions leave no hole, so the center is part of the shell.
	CHECK_MESSAGE(TEXT("Center should be inside the shell when MinimumDimensions is zero"),
		FNOrientedBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, FVector::ZeroVector, FVector(20.f), FRotator::ZeroRotator, FVector::ZeroVector));
}

N_TEST_HIGH(FNOrientedBoxPickerTests_IsPointInside_Strict, "NEXUS::UnitTests::NPicker::FNOrientedBoxPicker::IsPointInside_Strict", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Point strictly inside should be inside"),
		FNOrientedBoxPicker::IsPointInside(FVector::ZeroVector, FVector(20.f), FRotator::ZeroRotator, FVector(5.f, 0.f, 0.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point on the surface should not be strictly inside"),
		FNOrientedBoxPicker::IsPointInside(FVector::ZeroVector, FVector(20.f), FRotator::ZeroRotator, FVector(10.f, 0.f, 0.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point outside should not be strictly inside"),
		FNOrientedBoxPicker::IsPointInside(FVector::ZeroVector, FVector(20.f), FRotator::ZeroRotator, FVector(20.f, 0.f, 0.f)));
}

#endif //WITH_TESTS
