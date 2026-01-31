// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPickerParams.h"

/**
 * Provides various functions for generating points inside or on the surface of the FBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/box/">FNBoxPicker</a>
 */
class NEXUSPICKER_API FNBoxPicker
{
public:
	
	/**
	 * Generates a deterministic point in relation to an FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params);

	/**
	 * Generates a random point in relation to an FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params);
	

	/**
	 * Generates a random point in relation to an FBox.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNBoxPickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
	 	Tracked(OutLocations, DuplicateSeed, Params);
	}
	
	/**
	 * Generates a random point in relation to an FBox.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNBoxPickerParams& Params);

	/**
	 * Checks if a point is inside or on the surface of the FBox.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the FBox, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Dimensions, const FVector& Point)
	{
		const FBox Box(Origin + Dimensions.Min, Origin + Dimensions.Max);
		return Box.IsInsideOrOn(Point);
	}

	/**
	 * Checks if multiple points are inside or on the surface of the FBox.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions of the FBox.
	 * @param MaximumDimensions The maximum dimensions of the FBox.
	 * @return An array of boolean values indicating if each point is inside or on the surface of the FBox.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			if (IsPointInsideOrOn(Origin, MinimumDimensions, Point) || !IsPointInsideOrOn(Origin, MaximumDimensions, Point))
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