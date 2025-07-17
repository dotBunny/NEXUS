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
	FORCEINLINE static void NextPointOnProjected(FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::Deterministic.FloatRange(0, SplineComponent->Duration));
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	FORCEINLINE static void RandomPointOn(FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::NonDeterministic.FRandRange(0, SplineComponent->Duration));
	}
	FORCEINLINE static void RandomPointOnProjected(FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::NonDeterministic.FRandRange(0, SplineComponent->Duration));
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	FORCEINLINE static void RandomOneShotPointOn(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = SplineComponent->GetWorldLocationAtTime(RandomStream.FRandRange(0, SplineComponent->Duration));
	}
	FORCEINLINE static void RandomOneShotPointOnProjected(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointOn(Seed, OutLocation, SplineComponent);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	FORCEINLINE static void RandomTrackedPointOn(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = SplineComponent->GetWorldLocationAtTime(RandomStream.FRandRange(0, SplineComponent->Duration));
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedPointOnProjected(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointOn(Seed, OutLocation, SplineComponent);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	FORCEINLINE static bool IsPointOn(const USplineComponent* SplineComponent, const FVector& Point)
	{
		if (!SplineComponent)
		{
			return false;
		}

		const FVector ClosestLocationOnSpline = SplineComponent->FindLocationClosestToWorldLocation(Point, ESplineCoordinateSpace::World);
		const float Distance = FVector::Distance(Point, ClosestLocationOnSpline);
		return Distance <= N_PICKER_TOLERANCE;
	}

	// TODO: REMOVE
	FORCEINLINE static bool IsPointOnGrounded(const USplineComponent* SplineComponent, const FVector& Point)
	{
		if (!SplineComponent)
		{
			return false;
		}
		
		const float SplineLength = SplineComponent->GetSplineLength();
		const int32 NumOfPoints = FMath::RoundToInt(SplineLength / N_PICKER_TOLERANCE);
		const float DistancePerSegment = SplineLength / NumOfPoints;

		// Try to optimize for degenerate cases
		const FVector StartPosition = SplineComponent->GetLocationAtDistanceAlongSpline(0, ESplineCoordinateSpace::World);
		const FVector EndPosition = SplineComponent->GetLocationAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World);
		if (FVector::DistSquared(StartPosition, Point) <= FVector::DistSquared(EndPosition, Point))
		{
			// START -> END
			for (int32 i = 0; i <= NumOfPoints; i++)
			{
				const float Distance = DistancePerSegment * i;
				const FVector TestPoint = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
				if (FMath::Abs(TestPoint.X - Point.X) <= N_PICKER_TOLERANCE &&
					FMath::Abs(TestPoint.Y - Point.Y) <= N_PICKER_TOLERANCE)
				{
					return true;
				}
			}
		}
		else
		{
			// END -> START
			for (int32 i = NumOfPoints - 1; i >= 0; i--)
			{
				const float Distance = DistancePerSegment * i;
				const FVector TestPoint = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
				if (FMath::Abs(TestPoint.X - Point.X) <= N_PICKER_TOLERANCE &&
					FMath::Abs(TestPoint.Y - Point.Y) <= N_PICKER_TOLERANCE)
				{
					return true;
				}
			}
		}
		
		return false;
	}
};
