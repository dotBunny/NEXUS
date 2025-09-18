// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NRandom.h"
#include "Components/SplineComponent.h"

/**
 * Provides various functions for generating points along a USplineComponent spline using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/spline/">FNSplinePicker</a>
 */
class NEXUSPICKER_API FNSplinePicker
{
public:
	/**
	 * Generates a deterministic point on a spline.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param SplineComponent The spline component to generate points on.
	 */
	FORCEINLINE static void NextPointOn(FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::Deterministic.FloatRange(0, SplineComponent->Duration));
		N_IMPLEMENT_VLOG_SPLINE()
	}

	/**
	 * Generates a deterministic point on a spline, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param SplineComponent The spline component to generate points on.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointOnProjected(FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::Deterministic.FloatRange(0, SplineComponent->Duration));
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_SPLINE_PROJECTION()
		
	}
	
	/**
	 * Generates a random point on a spline.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param SplineComponent The spline component to generate points on.
	 */
	FORCEINLINE static void RandomPointOn(FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::NonDeterministic.FRandRange(0, SplineComponent->Duration));
		N_IMPLEMENT_VLOG_SPLINE()
	}

	/**
	 * Generates a random point on a spline, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param SplineComponent The spline component to generate points on.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointOnProjected(FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		OutLocation = SplineComponent->GetWorldLocationAtTime(FNRandom::NonDeterministic.FRandRange(0, SplineComponent->Duration));
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_SPLINE_PROJECTION()
	}
	
	/**
	 * Generates a random point on a spline using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param SplineComponent The spline component to generate points on.
	 */
	FORCEINLINE static void RandomOneShotPointOn(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = SplineComponent->GetWorldLocationAtTime(RandomStream.FRandRange(0, SplineComponent->Duration));
		N_IMPLEMENT_VLOG_SPLINE()
	}

	/**
	 * Generates a random point on a spline using a provided seed, then projects it to the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param SplineComponent The spline component to generate points on.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointOnProjected(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointOn(Seed, OutLocation, SplineComponent);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_SPLINE_PROJECTION()
	}
	
	/**
	 * Generates a random point on a spline while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param SplineComponent The spline component to generate points on.
	 */
	FORCEINLINE static void RandomTrackedPointOn(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = SplineComponent->GetWorldLocationAtTime(RandomStream.FRandRange(0, SplineComponent->Duration));
		Seed = RandomStream.GetCurrentSeed();
		N_IMPLEMENT_VLOG_SPLINE()
	}

	/**
	 * Generates a random point on a spline while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param SplineComponent The spline component to generate points on.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointOnProjected(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointOn(Seed, OutLocation, SplineComponent);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_SPLINE_PROJECTION()
	}

	/**
	 * Checks if a point is on a spline within a specified tolerance.
	 * Uses the N_PICKER_TOLERANCE defined in NPickerUtils.h for proximity checking.
	 * 
	 * @param SplineComponent The spline component to check against.
	 * @param Point The point to check.
	 * @return True if the point is on the spline within the tolerance, false otherwise.
	 */
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
};