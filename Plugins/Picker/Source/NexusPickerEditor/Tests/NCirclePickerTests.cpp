// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NCirclePicker.h"
#include "NCirclePickerParams.h"
#include "Macros/NTestMacros.h"
#include "Math/NMersenneTwister.h"

namespace NEXUS::UnitTests::NPicker::FNCirclePickerHarness
{
	constexpr int32 DensitySampleCount = 4000;
	constexpr int32 DensityBinCount = 5;
	constexpr float DensityBinTolerance = 0.20f;

	// Bins points by squared radius from Origin in the picker's XY plane. Each bin spans an equal
	// slice of (MaxR² - MinR²), so the bin areas are equal and an area-uniform sampler should
	// drop ~N/BinCount points in each one. A radius-uniform sampler clusters near MinimumRadius
	// and fails the inner-bin count by a wide margin.
	inline void CheckAreaUniformity(const FString& Label, const TArray<FVector>& Points,
		const FVector& Origin, const float MinRadius, const float MaxRadius)
	{
		const float MinRSq = MinRadius * MinRadius;
		const float MaxRSq = MaxRadius * MaxRadius;
		const float BinStep = (MaxRSq - MinRSq) / DensityBinCount;
		TArray<int32> BinCounts;
		BinCounts.Init(0, DensityBinCount);
		int32 Binned = 0;
		for (const FVector& Point : Points)
		{
			const float DX = static_cast<float>(Point.X - Origin.X);
			const float DY = static_cast<float>(Point.Y - Origin.Y);
			const float RSq = DX * DX + DY * DY;
			if (RSq < MinRSq - 0.01f || RSq > MaxRSq + 0.01f) continue;
			int32 Index = FMath::FloorToInt((RSq - MinRSq) / BinStep);
			Index = FMath::Clamp(Index, 0, DensityBinCount - 1);
			BinCounts[Index]++;
			Binned++;
		}

		CHECK_MESSAGE(FString::Printf(TEXT("%s: every point must land inside the annulus (binned %d of %d)"),
			*Label, Binned, Points.Num()), Binned == Points.Num());

		const float ExpectedPerBin = static_cast<float>(Points.Num()) / DensityBinCount;
		const float MaxDeviation = ExpectedPerBin * DensityBinTolerance;
		for (int32 i = 0; i < DensityBinCount; ++i)
		{
			CHECK_MESSAGE(FString::Printf(
				TEXT("%s: equal-area bin[%d] holds %d (expected %.0f ± %.0f) - radial density should be area-uniform"),
				*Label, i, BinCounts[i], ExpectedPerBin, MaxDeviation),
				FMath::Abs(BinCounts[i] - ExpectedPerBin) <= MaxDeviation);
		}
	}
}

N_TEST_CRITICAL(FNCirclePickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNCirclePicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 100.f;
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNCirclePicker::Next(Points, Twister, Params);
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
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNCirclePicker::Next(Points, Twister, Params);
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

N_TEST_HIGH(FNCirclePickerTests_Random_DensityUniformity, "NEXUS::UnitTests::NPicker::FNCirclePicker::Random_DensityUniformity", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies Random's annular distribution is area-uniform on the FRand-based path.
	using namespace NEXUS::UnitTests::NPicker::FNCirclePickerHarness;
	FNCirclePickerParams Params;
	Params.Origin = FVector(5.f, -7.f, 0.f);
	Params.Count = DensitySampleCount;
	Params.MinimumRadius = 10.f;
	Params.MaximumRadius = 100.f;
	Params.Rotation = FRotator::ZeroRotator;
	TArray<FVector> Points;
	FNCirclePicker::Random(Points, Params);
	CheckAreaUniformity(TEXT("Random"), Points, Params.Origin, Params.MinimumRadius, Params.MaximumRadius);
}

N_TEST_HIGH(FNCirclePickerTests_Next_DensityUniformity, "NEXUS::UnitTests::NPicker::FNCirclePicker::Next_DensityUniformity", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the Mersenne-Twister path also samples uniformly by area. Next has its own
	// RNG instance, so a regression in the FNMersenneTwister::Float pipeline would only show up here.
	using namespace NEXUS::UnitTests::NPicker::FNCirclePickerHarness;
	FNCirclePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = DensitySampleCount;
	Params.MinimumRadius = 10.f;
	Params.MaximumRadius = 100.f;
	Params.Rotation = FRotator::ZeroRotator;
	FNMersenneTwister Twister(0xC11C1E);
	TArray<FVector> Points;
	FNCirclePicker::Next(Points, Twister, Params);
	CheckAreaUniformity(TEXT("Next"), Points, Params.Origin, Params.MinimumRadius, Params.MaximumRadius);
}

#endif //WITH_TESTS
