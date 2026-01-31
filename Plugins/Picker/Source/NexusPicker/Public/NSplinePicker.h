// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"
#include "NSplinePickerParams.h"
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
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNSplinePickerParams& Params);

	/**
	 * Generates a deterministic point on a spline.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNSplinePickerParams& Params);

	/**
	 * Generates a deterministic point on a spline.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNSplinePickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
		Tracked(OutLocations, DuplicateSeed, Params);
	}

	/**
	 * Generates a deterministic point on a spline.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSplinePickerParams& Params);	

	/**
	 * Checks if a point is on a spline within a specified tolerance.
	 * Uses the N_PICKER_TOLERANCE defined in NPickerUtils.h for proximity checking.
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
		return Distance <= NEXUS::Picker::Tolerance;
	}
	
	/**
	 * Checks if multiple points are on a spline within a specified tolerance.
	 * @param Points The array of points to check.
	 * @param SplineComponent The spline component to check against.
	 * @return An array of boolean values indicating if each point is on a spline within a specified tolerance.
	 */
	FORCEINLINE static TArray<bool> IsPointsOn(const TArray<FVector>& Points, const USplineComponent* SplineComponent)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			OutResults.Add(IsPointOn(SplineComponent, Point));
		}
		return MoveTemp(OutResults);
	}
};