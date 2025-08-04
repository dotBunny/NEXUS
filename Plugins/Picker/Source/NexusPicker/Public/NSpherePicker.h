// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NRandom.h"

/** 
 * Provides various functions for generating points inside or on the surface of a sphere using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 */
class NEXUSPICKER_API FNSpherePicker
{
public:

	// NEXT POINT
	
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 */
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		OutLocation = Origin + (FNRandom::Deterministic.VRand() * FNRandom::Deterministic.FloatRange(MinimumRadius, MaximumRadius));
	}
	/**
	 * Generates a deterministic point inside or on the surface of a sphere, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 */
	FORCEINLINE static void NextPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const float Radius)
	{
		OutLocation = Origin + (FNRandom::Deterministic.VRand() * Radius);
	}
	/**
	 * Generates a deterministic point inside or on the surface of a sphere, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const float Radius, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOnSimple(OutLocation, Origin, Radius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	// RANDOM POINT

	/**
	 * Generates a random point inside or on the surface of a sphere.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 */
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		OutLocation = Origin + (FNRandom::NonDeterministic.VRand() * FNRandom::NonDeterministic.FRandRange(MinimumRadius, MaximumRadius));
	}
	/**
	 * Generates a random point inside or on the surface of a sphere, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	/**
	 * Generates a random point inside or on the surface of a sphere.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 */
	FORCEINLINE static void RandomPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const float Radius)
	{
		OutLocation = Origin + (FNRandom::NonDeterministic.VRand() * Radius);
	}
	/**
	 * Generates a random point inside or on the surface of a sphere, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const float Radius, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOnSimple(OutLocation, Origin, Radius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	// RANDOM ONE-SHOT POINT
	
	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * RandomStream.FRandRange(MinimumRadius, MaximumRadius));
	}
	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed, then projects it to the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnSimple(const int32 Seed, FVector& OutLocation, const FVector& Origin, const float Radius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * Radius);
	}
	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed, then projects it to the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnSimpleProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const float Radius, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOnSimple(Seed, OutLocation, Origin, Radius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	// RANDOM TRACKED POINT
	
	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * RandomStream.FRandRange(MinimumRadius, MaximumRadius));
		Seed = RandomStream.GetCurrentSeed();
	}
	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * @notes Projected points are not constrained by the MinimumRadius, nor MaximumRadius.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnSimple(int32& Seed, FVector& OutLocation, const FVector& Origin, const float Radius)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + (RandomStream.VRand() * Radius);
		Seed = RandomStream.GetCurrentSeed();
	}
	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * @notes Projected points are not constrained by the MinimumRadius, nor MaximumRadius.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnSimpleProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const float Radius, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOnSimple(Seed, OutLocation, Origin, Radius);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	// ASSERT
	
	/**
	 * Checks if a point is inside or on the surface of a sphere.
	 * 
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the sphere, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return (Origin - Point).Length() <= Radius;
	}
};