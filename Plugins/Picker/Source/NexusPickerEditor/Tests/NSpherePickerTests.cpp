// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NSpherePicker.h"
#include "NSpherePickerParams.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNSpherePickerTests_Next_PointCount, "NEXUS::UnitTests::NPicker::FNSpherePicker::Next_PointCount", N_TEST_CONTEXT_ANYWHERE)
{
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 50;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 100.f;
	TArray<FVector> Points;
	FNSpherePicker::Next(Points, Params);
	CHECK_MESSAGE(TEXT("Next should generate the requested number of points"), Points.Num() == 50);
}

N_TEST_CRITICAL(FNSpherePickerTests_Next_PointsInsideSphere, "NEXUS::UnitTests::NPicker::FNSpherePicker::Next_PointsInsideSphere", N_TEST_CONTEXT_ANYWHERE)
{
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 50.f;
	TArray<FVector> Points;
	FNSpherePicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Point[%d] should be inside the sphere"), i),
			FNSpherePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumRadius, Points[i]));
	}
}

N_TEST_HIGH(FNSpherePickerTests_IsPointInsideOrOn_Center, "NEXUS::UnitTests::NPicker::FNSpherePicker::IsPointInsideOrOn_Center", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Center should be inside sphere"),
		FNSpherePicker::IsPointInsideOrOn(FVector::ZeroVector, 10.f, FVector::ZeroVector));
}

N_TEST_HIGH(FNSpherePickerTests_IsPointInsideOrOn_OnSurface, "NEXUS::UnitTests::NPicker::FNSpherePicker::IsPointInsideOrOn_OnSurface", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Point exactly at radius should be on sphere"),
		FNSpherePicker::IsPointInsideOrOn(FVector::ZeroVector, 10.f, FVector(10.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNSpherePickerTests_IsPointInsideOrOn_Outside, "NEXUS::UnitTests::NPicker::FNSpherePicker::IsPointInsideOrOn_Outside", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Point beyond radius should be outside"),
		FNSpherePicker::IsPointInsideOrOn(FVector::ZeroVector, 10.f, FVector(100.f, 0.f, 0.f)));
}

N_TEST_HIGH(FNSpherePickerTests_Tracked_Determinism, "NEXUS::UnitTests::NPicker::FNSpherePicker::Tracked_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 20;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 50.f;

	int32 SeedA = 777;
	int32 SeedB = 777;
	TArray<FVector> PointsA, PointsB;
	FNSpherePicker::Tracked(PointsA, SeedA, Params);
	FNSpherePicker::Tracked(PointsB, SeedB, Params);
	for (int32 i = 0; i < PointsA.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Tracked sphere point[%d] should be deterministic"), i),
			PointsA[i].Equals(PointsB[i], 0.001f));
	}
}

N_TEST_HIGH(FNSpherePickerTests_Random_PointsInsideSphere, "NEXUS::UnitTests::NPicker::FNSpherePicker::Random_PointsInsideSphere", N_TEST_CONTEXT_ANYWHERE)
{
	FNSpherePickerParams Params;
	Params.Origin = FVector(5.f, 10.f, 15.f);
	Params.Count = 100;
	Params.MinimumRadius = 0.f;
	Params.MaximumRadius = 25.f;
	TArray<FVector> Points;
	FNSpherePicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Random sphere point[%d] should be inside sphere"), i),
			FNSpherePicker::IsPointInsideOrOn(Params.Origin, Params.MaximumRadius, Points[i]));
	}
}

N_TEST_HIGH(FNSpherePickerTests_IsPointsInsideOrOn_Shell, "NEXUS::UnitTests::NPicker::FNSpherePicker::IsPointsInsideOrOn_Shell", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that IsPointsInsideOrOn correctly rejects points inside the inner shell.
	TArray<FVector> Points;
	Points.Add(FVector::ZeroVector);
	Points.Add(FVector(7.f, 0.f, 0.f));
	Points.Add(FVector(100.f, 0.f, 0.f));

	const TArray<bool> Results = FNSpherePicker::IsPointsInsideOrOn(Points, FVector::ZeroVector, 5.f, 10.f);
	CHECK_FALSE_MESSAGE(TEXT("Origin should be outside the shell (inside inner radius)"), Results[0]);
	CHECK_MESSAGE(TEXT("Point in shell should be inside"), Results[1]);
	CHECK_FALSE_MESSAGE(TEXT("Point beyond outer radius should be outside"), Results[2]);
}

N_TEST_HIGH(FNSpherePickerTests_Random_PointsInsideShell, "NEXUS::UnitTests::NPicker::FNSpherePicker::Random_PointsInsideShell", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that Random respects MinimumRadius. Without this case, a regression where Random
	// ignored the inner radius would slip through Random_PointsInsideSphere (which uses Min=0).
	FNSpherePickerParams Params;
	Params.Origin = FVector(2.f, -4.f, 1.f);
	Params.Count = 200;
	Params.MinimumRadius = 10.f;
	Params.MaximumRadius = 25.f;

	TArray<FVector> Points;
	FNSpherePicker::Random(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		const float Distance = static_cast<float>(FVector::Dist(Points[i], Params.Origin));
		CHECK_MESSAGE(FString::Printf(TEXT("Random shell point[%d] (%.2f) should sit inside [%.2f, %.2f]"), i, Distance, Params.MinimumRadius, Params.MaximumRadius),
			Distance >= Params.MinimumRadius - 0.001f && Distance <= Params.MaximumRadius + 0.001f);
	}
}

N_TEST_HIGH(FNSpherePickerTests_Next_PointsInsideShell, "NEXUS::UnitTests::NPicker::FNSpherePicker::Next_PointsInsideShell", N_TEST_CONTEXT_ANYWHERE)
{
	// Mirror Random coverage on the deterministic Next path with a non-zero MinimumRadius.
	FNSpherePickerParams Params;
	Params.Origin = FVector::ZeroVector;
	Params.Count = 200;
	Params.MinimumRadius = 8.f;
	Params.MaximumRadius = 20.f;

	TArray<FVector> Points;
	FNSpherePicker::Next(Points, Params);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		const float Distance = static_cast<float>(FVector::Dist(Points[i], Params.Origin));
		CHECK_MESSAGE(FString::Printf(TEXT("Next shell point[%d] (%.2f) should sit inside [%.2f, %.2f]"), i, Distance, Params.MinimumRadius, Params.MaximumRadius),
			Distance >= Params.MinimumRadius - 0.001f && Distance <= Params.MaximumRadius + 0.001f);
	}
}

#endif //WITH_TESTS
