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

		const FVector ClosestLocationOnSpline = SplineComponent->FindLocationClosestToWorldLocation(Point, ESplineCoordinateSpace::World);
		const float Distance = FVector::Distance(Point, ClosestLocationOnSpline);
		if (Distance > N_PICKER_TOLERANCE)
		{
			N_LOG(Warning, TEXT("%s : %s : %f"), *ClosestLocationOnSpline.ToCompactString(), *Point.ToCompactString(), Distance)
		}
		return Distance <= N_PICKER_TOLERANCE;
	}

	FORCEINLINE static bool IsPointOnGrounded(const USplineComponent* SplineComponent, const FVector& Point)
	{
		if (!SplineComponent)
		{
			return false;
		}
		
		const float SplineLength = SplineComponent->GetSplineLength();
		const int32 NumOfPoints = FMath::RoundToInt(SplineLength / N_PICKER_TOLERANCE);
		const float DistancePerSegment = SplineLength / NumOfPoints;
		
		for (int32 i = 0; i <= NumOfPoints; ++i)
		{
			const float Distance = DistancePerSegment * i;
			const FVector TestPoint = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			if (FMath::Abs(TestPoint.X - Point.X) <= N_PICKER_TOLERANCE &&
				FMath::Abs(TestPoint.Y - Point.Y) <= N_PICKER_TOLERANCE)
			{
				return true;
			}
		}
		return false;
	}
};
