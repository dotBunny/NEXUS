// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NRandom.h"

/**
 * Methods for selecting points related to a sphere.
 */
class NEXUSPICKER_API FNSpherePicker
{
public:
	FORCEINLINE static void NextPointInside(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		OutLocation = Origin + (FNRandom::Deterministic.VRand() * FNRandom::Deterministic.FloatRange(MinimumRadius, MaximumRadius));
	}
	FORCEINLINE static void NextGroundedPointInside(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		NextPointInside(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomPointInside(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		OutLocation = Origin + (FNRandom::NonDeterministic.VRand() * FNRandom::NonDeterministic.FRandRange(MinimumRadius, MaximumRadius));
	}
	FORCEINLINE static void RandomGroundedPointInside(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomPointInside(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	FORCEINLINE static void RandomOneShotPointInside(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * RandomStream.FRandRange(MinimumRadius, MaximumRadius));
	}
	FORCEINLINE static void RandomOneShotGroundedPointInside(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomOneShotPointInside(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	
	FORCEINLINE static void RandomTrackedPointInside(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * RandomStream.FRandRange(MinimumRadius, MaximumRadius));
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedGroundedPointInside(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomTrackedPointInside(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}

	FORCEINLINE static bool IsPointInside(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return (Origin - Point).Length() <= Radius;
	}
};
