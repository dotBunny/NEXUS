// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NBoxPicker.h"
#include "NBoxPickerParams.h"
#include "Macros/NTestMacros.h"
#include "Math/NMersenneTwister.h"

N_TEST_CRITICAL(FNBoxPickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNBoxPicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MaximumBox = FBox(FVector(-100.f), FVector(100.f));
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNBoxPicker::Next(Points, Twister, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNBoxPickerTests_Next_PointsInsideBounds, "NEXUS::UnitTests::NPicker::FNBoxPicker::Next_PointsInsideBounds", N_TEST_CONTEXT_ANYWHERE)
{
	FNBoxPickerParams Params;
	Params.Origin = FVector(50.f, 50.f, 50.f);
	Params.Count = 200;
	Params.MaximumBox = FBox(FVector(-10.f), FVector(10.f));
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNBoxPicker::Next(Points, Twister, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the box"), i),
			FNBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumBox, Points[i]));
	}
}

N_TEST_HIGH(FNBoxPickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNBoxPicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 20;
	Params.MaximumBox = FBox(FVector(-50.f), FVector(50.f));

	int32 SeedA = 42;
	int32 SeedB = 42;
	TArray<FVector> PointsA, PointsB;
	FNBoxPicker::Tracked(PointsA, SeedA, Params);
	FNBoxPicker::Tracked(PointsB, SeedB, Params);

	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked point[%d] should be deterministic with same seed"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
}

N_TEST_HIGH(FNBoxPickerTests_Tracked_SeedMutates, "NEXUS::UnitTests::NPicker::FNBoxPicker::Tracked_SeedMutates", N_TEST_CONTEXT_ANYWHERE)
{
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 5;
	Params.MaximumBox = FBox(FVector(-10.f), FVector(10.f));

	int32 Seed = 12345;
	const int32 OriginalSeed = Seed;
	TArray<FVector> Points;
	FNBoxPicker::Tracked(Points, Seed, Params);
	CHECK_MESSAGE(TEXT("Tracked should mutate the seed"), Seed != OriginalSeed);
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_Inside, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_Inside", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector Origin(0.f);
	const FBox Box(FVector(-10.f), FVector(10.f));
	CHECK_MESSAGE(TEXT("Origin should be inside box"), FNBoxPicker::IsPointInsideOrOn(Origin, Box, FVector::ZeroVector));
	CHECK_MESSAGE(TEXT("Corner should be on box"), FNBoxPicker::IsPointInsideOrOn(Origin, Box, FVector(10.f)));
}

N_TEST_CRITICAL(FNBoxPickerTests_Next_PointsInsideBounds_OffsetBox, "NEXUS::UnitTests::NPicker::FNBoxPicker::Next_PointsInsideBounds_OffsetBox", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that points generated from a box not centered at local zero pass their own inclusion test.
	FNBoxPickerParams Params;
	Params.Origin = FVector(1000.f, 0.f, 0.f);
	Params.Count = 200;
	Params.MaximumBox = FBox(FVector(0.f), FVector(100.f));
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNBoxPicker::Next(Points, Twister, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the offset box"), i),
			FNBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumBox, Points[i]));
	}
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_OffsetBox, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_OffsetBox", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that the box is treated as an offset region relative to Origin, not re-centered on it.
	const FVector Origin(1000.f, 0.f, 0.f);
	const FBox Box(FVector(0.f), FVector(100.f));
	CHECK_MESSAGE(TEXT("Shifted minimum corner should be on box"), FNBoxPicker::IsPointInsideOrOn(Origin, Box, FVector(1000.f, 0.f, 0.f)));
	CHECK_MESSAGE(TEXT("Shifted maximum corner should be on box"), FNBoxPicker::IsPointInsideOrOn(Origin, Box, FVector(1100.f, 100.f, 100.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point inside the re-centered region should not be inside the shifted box"), FNBoxPicker::IsPointInsideOrOn(Origin, Box, FVector(960.f, -10.f, -10.f)));
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_Outside, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_Outside", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector Origin(0.f);
	const FBox Box(FVector(-10.f), FVector(10.f));
	CHECK_FALSE_MESSAGE(TEXT("Point far away should not be inside"), FNBoxPicker::IsPointInsideOrOn(Origin, Box, FVector(100.f)));
}

N_TEST_HIGH(FNBoxPickerTests_Random_PointsInsideBounds, "NEXUS::UnitTests::NPicker::FNBoxPicker::Random_PointsInsideBounds", N_TEST_CONTEXT_ANYWHERE)
{
	FNBoxPickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 100;
	Params.MaximumBox = FBox(FVector(-25.f), FVector(25.f));
	TArray<FVector> Points;
	FNBoxPicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random point[%d] should be inside the box"), i),
			FNBoxPicker::IsPointInsideOrOn(Params.Origin, Params.MaximumBox, Points[i]));
	}
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_Shell_InnerBoundary, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_Shell_InnerBoundary", N_TEST_CONTEXT_ANYWHERE)
{
	// A point exactly on the inner surface is part of the closed shell.
	const FBox MinBox(FVector(-5.f), FVector(5.f));
	const FBox MaxBox(FVector(-10.f), FVector(10.f));
	CHECK_MESSAGE(TEXT("Point on the inner surface should be inside the shell"),
		FNBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, MinBox, MaxBox, FVector(5.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_Shell_OuterBoundary, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_Shell_OuterBoundary", N_TEST_CONTEXT_ANYWHERE)
{
	const FBox MinBox(FVector(-5.f), FVector(5.f));
	const FBox MaxBox(FVector(-10.f), FVector(10.f));
	CHECK_MESSAGE(TEXT("Point on the outer surface should be inside the shell"),
		FNBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, MinBox, MaxBox, FVector(10.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_Shell_Hole, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_Shell_Hole", N_TEST_CONTEXT_ANYWHERE)
{
	const FBox MinBox(FVector(-5.f), FVector(5.f));
	const FBox MaxBox(FVector(-10.f), FVector(10.f));
	CHECK_FALSE_MESSAGE(TEXT("Point strictly inside the inner box should be excluded"),
		FNBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, MinBox, MaxBox, FVector(2.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInsideOrOn_Shell_NoHoleIncludesCenter, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInsideOrOn_Shell_NoHoleIncludesCenter", N_TEST_CONTEXT_ANYWHERE)
{
	// A zero-size inner box leaves no hole, so the center is part of the shell.
	const FBox MinBox(FVector::ZeroVector, FVector::ZeroVector);
	const FBox MaxBox(FVector(-10.f), FVector(10.f));
	CHECK_MESSAGE(TEXT("Center should be inside the shell when the inner box is empty"),
		FNBoxPicker::IsPointInsideOrOn(FVector::ZeroVector, MinBox, MaxBox, FVector::ZeroVector));
}

N_TEST_HIGH(FNBoxPickerTests_IsPointInside_Strict, "NEXUS::UnitTests::NPicker::FNBoxPicker::IsPointInside_Strict", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector Origin(0.f);
	const FBox Box(FVector(-10.f), FVector(10.f));
	CHECK_MESSAGE(TEXT("Point strictly inside should be inside"),
		FNBoxPicker::IsPointInside(Origin, Box, FVector(5.f, 0.f, 0.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point on the surface should not be strictly inside"),
		FNBoxPicker::IsPointInside(Origin, Box, FVector(10.f, 0.f, 0.f)));
	CHECK_FALSE_MESSAGE(TEXT("Point outside should not be strictly inside"),
		FNBoxPicker::IsPointInside(Origin, Box, FVector(20.f, 0.f, 0.f)));
}

#endif //WITH_TESTS
