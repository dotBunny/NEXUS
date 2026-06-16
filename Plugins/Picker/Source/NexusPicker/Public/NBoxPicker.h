// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPickerParams.h"
#include "Math/NMersenneTwister.h"

/**
 * Provides various functions for generating points inside or on the surface of the FBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/box/">FNBoxPicker</a>
 */
class NEXUSPICKER_API FNBoxPicker
{
public:
	
	/**
	 * Generate random points in relation to an axis-aligned FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 * @note Not thread-safe; all pickers share a single non-deterministic FRandomStream (FNRandom::GetNonDeterministic()). Only call from the Game-thread.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params);
	
	/**
	 * Generate random points in relation to an axis-aligned FBox.
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
	 * Generate random points in relation to an axis-aligned FBox.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNBoxPickerParams& Params);

	/**
	 * Generate random points in relation to an axis-aligned FBox using a provided Mersenne Twister.	 
	 * @param OutLocations An array to store the generated points.
	 * @param Random The Mersenne Twister to query for random.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNBoxPickerParams& Params);
	
	/**
	 * Checks if a point is inside or on the surface of the axis-aligned FBox.
	 * @param Origin The world-space origin the FBox is offset from.
	 * @param Box The FBox to check the point against.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the FBox, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Box, const FVector& Point)
	{
		return Box.ShiftBy(Origin).IsInsideOrOn(Point);
	}

	/**
	 * Checks if multiple points are inside or on the surface of the axis-aligned FBox.
	 * @param Points The array of points to check.
	 * @param Origin The world-space origin the boxes are offset from.
	 * @param MinimumBox The minimum FBox.
	 * @param MaximumBox The maximum FBox.
	 * @return An array of boolean values indicating if each point is inside or on the surface of the FBox.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FBox& MinimumBox, const FBox& MaximumBox)
	{
		TArray<bool> OutResults;
		OutResults.Reserve(Points.Num());
		
		for (const FVector& Point : Points)
		{
			const bool bValid = IsPointInsideOrOn(Origin, MaximumBox, Point)
							 && !IsPointInsideOrOn(Origin, MinimumBox, Point);
			OutResults.Add(bValid);
		}
		return OutResults;
	}
};