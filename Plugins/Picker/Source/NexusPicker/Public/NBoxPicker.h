// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

/**
 * Methods for selecting points related to a box.
 */
class NEXUSPICKER_API FNBoxPicker
{
public:
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::Deterministic.FloatRange(Dimensions.Min.X, Dimensions.Max.X),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Y, Dimensions.Max.Y),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	FORCEINLINE static void NextGroundedPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_GROUNDED())
	{
		NextPointInsideOrOn(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the dimensions
		if (!IsPointInsideOrOn(Origin, Dimensions, OutLocation))
		{
			OutLocation = FBox(Origin + Dimensions.Min, Origin + Dimensions.Max).GetClosestPointTo(OutLocation);
		}
	}
	
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::NonDeterministic.RandRange(Dimensions.Min.X, Dimensions.Max.X),
			FNRandom::NonDeterministic.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
			FNRandom::NonDeterministic.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	FORCEINLINE static void RandomGroundedPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_GROUNDED())
	{
		RandomPointInsideOrOn(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the dimensions
		if (!IsPointInsideOrOn(Origin, Dimensions, OutLocation))
		{
			OutLocation = FBox(Origin + Dimensions.Min, Origin + Dimensions.Max).GetClosestPointTo(OutLocation);
		}
	}
	
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.RandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	FORCEINLINE static void RandomOneShotGroundedPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the dimensions
		if (!IsPointInsideOrOn(Origin, Dimensions, OutLocation))
		{
			OutLocation = FBox(Origin + Dimensions.Min, Origin + Dimensions.Max).GetClosestPointTo(OutLocation);
		}
	}
	
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.RandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedGroundedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the dimensions
		if (!IsPointInsideOrOn(Origin, Dimensions, OutLocation))
		{
			OutLocation = FBox(Origin + Dimensions.Min, Origin + Dimensions.Max).GetClosestPointTo(OutLocation);
		}
	}

	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Dimensions, const FVector& Point)
	{
		const FBox Box(Origin + Dimensions.Min, Origin + Dimensions.Max);
		return Box.IsInsideOrOn(Point);
	}
};
