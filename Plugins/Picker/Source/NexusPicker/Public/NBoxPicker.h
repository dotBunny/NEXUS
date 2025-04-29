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
	FORCEINLINE static void NextPointInside(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::Deterministic.FloatRange(Dimensions.Min.X, Dimensions.Max.X),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Y, Dimensions.Max.Y),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	FORCEINLINE static void NextGroundedPointInside(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_GROUNDED())
	{
		NextPointInside(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomPointInside(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::NonDeterministic.RandRange(Dimensions.Min.X, Dimensions.Max.X),
			FNRandom::NonDeterministic.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
			FNRandom::NonDeterministic.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	FORCEINLINE static void RandomGroundedPointInside(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_GROUNDED())
	{
		RandomPointInside(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomOneShotPointInside(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.RandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
	}
	FORCEINLINE static void RandomOneShotGroundedPointInside(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomOneShotPointInside(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomTrackedPointInside(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.RandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.RandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.RandRange(Dimensions.Min.Z, Dimensions.Max.Z));
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedGroundedPointInside(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomTrackedPointInside(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
};
