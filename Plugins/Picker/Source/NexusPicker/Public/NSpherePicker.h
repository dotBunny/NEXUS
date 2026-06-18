// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePickerParams.h"
#include "Math/NMersenneTwister.h"

/** 
 * Provides various functions for generating points inside or on the surface of a sphere using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/">FNSpherePicker</a>
 */
class NEXUSPICKER_API FNSpherePicker
{
public:

	/**
	 * Generate random points inside or on the surface of a sphere.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 * @note Not thread-safe; all pickers share a single non-deterministic FRandomStream (FNRandom::GetNonDeterministic()). Only call from the Game-thread.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params);
	

	/**
	 * Generate random points inside or on the surface of a sphere.
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
	 * Generate random points inside or on the surface of a sphere.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSpherePickerParams& Params);	
	
	/**
	 * Generate random points inside or on the surface of a sphere using a provided Mersenne Twister.	 
	 * @param OutLocations An array to store the generated points.
	 * @param Random The Mersenne Twister to query for random.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNSpherePickerParams& Params);

	/**
	 * Checks if a point is inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the sphere, false otherwise.
	 * @note Inclusive: a point exactly on the surface returns true.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return FVector::Distance(Point, Origin) <= Radius;
	}

	/**
	 * Checks if a point is strictly inside a sphere, excluding the surface itself.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param Point The point to check.
	 * @return True if the point is strictly inside the sphere, false if on the surface or outside.
	 */
	FORCEINLINE static bool IsPointInside(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return FVector::Distance(Point, Origin) < Radius;
	}

	/**
	 * Checks if a point is inside or on a spherical shell.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param Point The point to check.
	 * @return True if the point is inside or on the spherical shell, false otherwise.
	 * @note Closed shell: points on the inner OR outer boundary are included; only points strictly inside MinimumRadius (the hole) are excluded. When MinimumRadius is 0 there is no hole, so the center is included.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FVector& Point)
	{
		return IsPointInsideOrOn(Origin, MaximumRadius, Point) && !IsPointInside(Origin, MinimumRadius, Point);
	}

	/**
	 * Checks if multiple points are inside or on a spherical shell.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return An array of boolean values indicating if each point is inside or on the spherical shell.
	 * @note Closed shell: points on the inner OR outer boundary are included; only points strictly inside MinimumRadius (the hole) are excluded. When MinimumRadius is 0 there is no hole, so every point within MaximumRadius (including the center) is included.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		TArray<bool> OutResults;
		OutResults.Reserve(Points.Num());

		for (const FVector& Point : Points)
		{
			OutResults.Add(IsPointInsideOrOn(Origin, MinimumRadius, MaximumRadius, Point));
		}
		return OutResults;
	}
};