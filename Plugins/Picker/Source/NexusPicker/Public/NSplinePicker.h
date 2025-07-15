// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NRandom.h"
#include "Components/SplineComponent.h"

/**
 * Methods for selecting points related to a spline.
 */
class NEXUSPICKER_API FNSplinePicker
{
public:
	FORCEINLINE static void NextPointOn(FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::Deterministic.FloatRange(0, SplineComponent->Duration));
	}
	FORCEINLINE static void NextGroundedPointOn(FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_GROUNDED())
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::Deterministic.FloatRange(0, SplineComponent->Duration));
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	
	FORCEINLINE static void RandomPointOn(FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::NonDeterministic.FRandRange(0, SplineComponent->Duration));
	}
	FORCEINLINE static void RandomGroundedPointOn(FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_GROUNDED())
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::NonDeterministic.FRandRange(0, SplineComponent->Duration));
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	
	FORCEINLINE static void RandomOneShotPointOn(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = SplineComponent->GetWorldLocationAtTime(RandomStream.FRandRange(0, SplineComponent->Duration));
	}
	FORCEINLINE static void RandomOneShotGroundedPointOn(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomOneShotPointOn(Seed, OutLocation, SplineComponent);
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	
	FORCEINLINE static void RandomTrackedPointOn(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = SplineComponent->GetWorldLocationAtTime(RandomStream.FRandRange(0, SplineComponent->Duration));
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedGroundedPointOn(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomTrackedPointOn(Seed, OutLocation, SplineComponent);
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	FORCEINLINE static bool IsPointOn(const USplineComponent* SplineComponent, const FVector& Point)
	{
		if (!SplineComponent)
		{
			return false;
		}
		
		const float Key = SplineComponent->FindInputKeyClosestToWorldLocation(Point);
		const FVector ClosestLocationOnSpline = SplineComponent->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);

		return FVector::Distance(Point, ClosestLocationOnSpline) < N_PICKER_TOLERANCE;
	}

	FORCEINLINE static bool IsPointOnGrounded(const USplineComponent* SplineComponent, const FVector& Point)
	{
		if (!SplineComponent)
		{
			return false;
		}
		const float Key = SplineComponent->FindInputKeyClosestToWorldLocation(Point);
		const FVector ClosestLocationOnSpline = SplineComponent->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
		const FVector AdjustedPoint = FVector(Point.X, Point.Y, ClosestLocationOnSpline.Z);
		return FVector::Distance(AdjustedPoint, ClosestLocationOnSpline) < N_PICKER_TOLERANCE;
	}
};
