// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrientedBoxPickerParams.h"

/**
 * Provides various functions for generating points inside or on the surface of the FOrientedBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/">FNBoxPicker</a>
 */
class NEXUSPICKER_API FNOrientedBoxPicker
{
public:
	
	/**
	 * Generates a deterministic point in relation to an FOrientedBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params);

	/**
	 * Generates a random point in relation to an FOrientedBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params);
	
	/**
	 * Generates a random point in relation to an FOrientedBox.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNOrientedBoxPickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
	 	Tracked(OutLocations, DuplicateSeed, Params);
	}
	
	/**
	 * Generates a random point in relation to an FOrientedBox.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNOrientedBoxPickerParams& Params);
	
	/**
	 * Checks if a point is inside or on the surface of the FOrientedBox.
	 * @param Origin The center point of the FOrientedBox.
	 * @param Dimensions The dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the FOrientedBox, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FVector& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		const FVector Extents = Dimensions * 0.5f;
		const float MinX =  Origin.X - Extents.X;
		const float MaxX =  Origin.X + Extents.X;
		const float MinY =  Origin.Y - Extents.Y;
		const float MaxY =  Origin.Y + Extents.Y;
		const float MinZ =  Origin.Z - Extents.Z;
		const float MaxZ =  Origin.Z + Extents.Z;
		
		const FVector Translated = Point - Origin;
		const FVector Unrotated = Rotation.UnrotateVector(Translated);
		const FVector Result = Unrotated + Origin;
		
		return (Result.X >= MinX && Result.X <= MaxX && 
			Result.Y >= MinY && Result.Y <= MaxY && 
			Result.Z >= MinZ && Result.Z <= MaxZ);
	}

	/**
	 * Checks if multiple points are inside or on the surface of the FOrientedBox.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the FOrientedBox.
	 * @param MinimumDimensions The minimum dimensions of the FOrientedBox.
	 * @param MaximumDimensions The maximum dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @return An array of boolean values indicating if each point is inside or on the surface of the FBox.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FVector& MinimumDimensions, const FVector& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			if (IsPointInsideOrOn(Origin, MinimumDimensions, Rotation, Point) || !IsPointInsideOrOn(Origin, MaximumDimensions, Rotation, Point))
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