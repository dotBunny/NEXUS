// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePickerParams.h"

/** 
 * Provides various functions for generating points inside or on the surface of a sphere using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/">FNSpherePicker</a>
 */
class NEXUSPICKER_API FNSpherePicker
{
public:

	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params);

	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params);
	

	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNSpherePickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
		Tracked(OutLocations, DuplicateSeed, Params);
	}

	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSpherePickerParams& Params);	
	
	/**
	 * Checks if a point is inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the sphere, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return FVector::Distance(Point, Origin) <= Radius;
	}
	
	/**
	 * Checks if multiple points are inside or on the surface of a sphere.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the FBox.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (inner bound).
	 * @return An array of boolean values indicating if each point is inside or on the surface of a sphere.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			if (IsPointInsideOrOn(Origin, MinimumRadius, Point) || !IsPointInsideOrOn(Origin, MaximumRadius, Point))
			{
				OutResults.Add(false);
			}
			else
			{
				OutResults.Add(true);
			}
		}
		return MoveTemp(OutResults);
	}
};