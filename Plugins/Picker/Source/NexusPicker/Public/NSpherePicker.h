// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCoreMinimal.h"
#include "NRandom.h"

/**
 * Methods for selecting points related to a sphere.
 */
class NEXUSPICKER_API FNSpherePicker
{
public:
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		OutLocation = Origin + (FNRandom::Deterministic.VRand() * FNRandom::Deterministic.FloatRange(MinimumRadius, MaximumRadius));
	}
	FORCEINLINE static void NextGroundedPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
		
		// Ensure the point is within the maximum radius
		if (!IsPointInsideOrOn(Origin, MaximumRadius, OutLocation))
		{
			FVector Direction = OutLocation - Origin;
			Direction.Normalize();
			OutLocation = Origin + (Direction * (MaximumRadius - N_GROUNDING_TOLERANCE));
		}
	}
	
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		OutLocation = Origin + (FNRandom::NonDeterministic.VRand() * FNRandom::NonDeterministic.FRandRange(MinimumRadius, MaximumRadius));
	}
	FORCEINLINE static void RandomGroundedPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the maximum radius
		if (!IsPointInsideOrOn(Origin, MaximumRadius, OutLocation))
		{
			FVector Direction = OutLocation - Origin;
			Direction.Normalize();
			OutLocation = Origin + (Direction * (MaximumRadius - N_GROUNDING_TOLERANCE));
		}
	}

	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * RandomStream.FRandRange(MinimumRadius, MaximumRadius));
	}
	FORCEINLINE static void RandomOneShotGroundedPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the maximum radius
		if (!IsPointInsideOrOn(Origin, MaximumRadius, OutLocation))
		{
			FVector Direction = OutLocation - Origin;
			Direction.Normalize();
			OutLocation = Origin + (Direction * (MaximumRadius - N_GROUNDING_TOLERANCE));
		}
	}

	
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * RandomStream.FRandRange(MinimumRadius, MaximumRadius));
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedGroundedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
		// Ensure the point is within the maximum radius
		if (!IsPointInsideOrOn(Origin, MaximumRadius, OutLocation))
		{
			FVector Direction = OutLocation - Origin;
			Direction.Normalize();
			OutLocation = Origin + (Direction * (MaximumRadius - N_GROUNDING_TOLERANCE));
		}
	}

	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return (Origin - Point).Length() <= Radius;
	}
};
