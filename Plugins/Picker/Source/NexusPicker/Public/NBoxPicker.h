// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

// TODO: Add Inner/Outer Dimension Option (#36)

/**
 * Provides various functions for generating points inside or on the surface of a FBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 */
class NEXUSPICKER_API FNBoxPicker
{
public:
	
	/**
	 * Generates a deterministic point inside or on the surface of a FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::Deterministic.FloatRange(Dimensions.Min.X, Dimensions.Max.X),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Y, Dimensions.Max.Y),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	
	/**
	 * Generates a deterministic point inside or on the surface of a FBox, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the surface of a FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::NonDeterministic.RandRange(Dimensions.Min.X, Dimensions.Max.X),
			FNRandom::NonDeterministic.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
			FNRandom::NonDeterministic.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	
	/**
	 * Generates a random point inside or on the surface of a FBox, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the surface of a FBox using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.RandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	
	/**
	 * Generates a random point inside or on the surface of a FBox using a provided seed, then projects it to the world.	 
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the surface of a FBox while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.RandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
		Seed = RandomStream.GetCurrentSeed();
	}
	
	/**
	 * Generates a random point inside or on the surface of a FBox while tracking the random seed state, then grounds it to the world.
	 * Updates the seed value to enable sequential random point generation.	
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	/**
	 * Checks if a point is inside or on the surface of a FBox.
	 * 
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
};