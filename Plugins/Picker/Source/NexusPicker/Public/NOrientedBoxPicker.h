// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NOrientedBoxPickerParams.h"
#include "Math/NMersenneTwister.h"

/**
 * Provides various functions for generating points inside or on the surface of the FOrientedBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/">FNOrientedBoxPicker</a>
 */
class NEXUSPICKER_API FNOrientedBoxPicker
{
public:

	/**
	 * Generate random points in relation to an FOrientedBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 * @note Not thread-safe; all pickers share a single non-deterministic FRandomStream (FNRandom::GetNonDeterministic()). Only call from the Game-thread.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params);

	/**
	 * Generate random points in relation to an FOrientedBox.
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
	 * Generate random points in relation to an FOrientedBox.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNOrientedBoxPickerParams& Params);

	/**
	 * Generate random points in relation to an FOrientedBox using a provided Mersenne Twister.
	 * @param OutLocations An array to store the generated points.
	 * @param Random The Mersenne Twister to query for random.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNOrientedBoxPickerParams& Params);

	/**
	 * Checks if a point is inside or on the surface of the FOrientedBox.
	 * @param Origin The center point of the FOrientedBox.
	 * @param Dimensions The dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the FOrientedBox, false otherwise.
	 * @note Inclusive: a point exactly on the surface returns true.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FVector& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		const FVector Translated = Point - Origin;
		const FVector Unrotated  = Rotation.UnrotateVector(Translated);
		const FVector Extents = Dimensions * 0.5f;
		return (FMath::Abs(Unrotated.X) <= Extents.X &&
				FMath::Abs(Unrotated.Y) <= Extents.Y &&
				FMath::Abs(Unrotated.Z) <= Extents.Z);
	}

	/**
	 * Checks if a point is strictly inside the FOrientedBox, excluding its surface.
	 * @param Origin The center point of the FOrientedBox.
	 * @param Dimensions The dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @param Point The point to check.
	 * @return True if the point is strictly inside the FOrientedBox, false if on the surface or outside.
	 */
	FORCEINLINE static bool IsPointInside(const FVector& Origin, const FVector& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		const FVector Translated = Point - Origin;
		const FVector Unrotated  = Rotation.UnrotateVector(Translated);
		const FVector Extents = Dimensions * 0.5f;
		return (FMath::Abs(Unrotated.X) < Extents.X &&
				FMath::Abs(Unrotated.Y) < Extents.Y &&
				FMath::Abs(Unrotated.Z) < Extents.Z);
	}

	/**
	 * Checks if a point is inside or on the shell between two concentric oriented boxes.
	 * @param Origin The center point of the FOrientedBox.
	 * @param MinimumDimensions The minimum dimensions of the FOrientedBox.
	 * @param MaximumDimensions The maximum dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the oriented-box shell, false otherwise.
	 * @note Closed shell: points on the inner OR outer surface are included; only points strictly inside MinimumDimensions (the hole) are excluded. When MinimumDimensions is zero there is no hole, so the center is included.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FVector& MinimumDimensions, const FVector& MaximumDimensions, const FRotator& Rotation, const FVector& Point)
	{
		return IsPointInsideOrOn(Origin, MaximumDimensions, Rotation, Point) && !IsPointInside(Origin, MinimumDimensions, Rotation, Point);
	}

	/**
	 * Checks if multiple points are inside or on the shell between two concentric oriented boxes.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the FOrientedBox.
	 * @param MinimumDimensions The minimum dimensions of the FOrientedBox.
	 * @param MaximumDimensions The maximum dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @return An array of boolean values indicating if each point is inside or on the oriented-box shell.
	 * @note Closed shell: points on the inner OR outer surface are included; only points strictly inside MinimumDimensions (the hole) are excluded. When MinimumDimensions is zero there is no hole, so every point inside or on MaximumDimensions (including the center) is included.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FVector& MinimumDimensions, const FVector& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		TArray<bool> OutResults;
		OutResults.Reserve(Points.Num());

		for (const FVector& Point : Points)
		{
			OutResults.Add(IsPointInsideOrOn(Origin, MinimumDimensions, MaximumDimensions, Rotation, Point));
		}
		return OutResults;
	}
};