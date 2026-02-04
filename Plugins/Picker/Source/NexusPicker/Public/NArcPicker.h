// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NArcPickerParams.h"

/**
 * Provides various functions for generating points as part of an arc using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/arc/">FNArcPicker</a>
 */
class NEXUSPICKER_API FNArcPicker
{
public:
	
	/**
	 * Generate deterministic points as part of an arc.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNArcPickerParams& Params);

	/**
	 * Generate random points as part of an arc.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNArcPickerParams& Params);

	/**
	 * Generate random points as part of an arc.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNArcPickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
	 	Tracked(OutLocations, DuplicateSeed, Params);
	}
	
	/**
	 * Generate random points as part of an arc.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNArcPickerParams& Params);
	
	
	/**
	 * Checks if a point is inside or on the surface of the axis-aligned FBox.
	 * @param Origin The center point of the FBox.
	 * @param Rotation The base rotation of the arc.
	 * @param Degrees The angle of the arc in degrees.
	 * @param MinimumDistance The minimum distance of the arc.
	 * @param MaximumDistance The maximum distance of the arc.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the FBox, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FRotator& Rotation, const float& Degrees, const float& MinimumDistance, const float& MaximumDistance, const FVector& Point)
	{
		const FVector Direction = Point - Origin;
		const float DistanceSquared = Direction.SizeSquared();
		if (DistanceSquared > (MaximumDistance * MaximumDistance) || 
			DistanceSquared < (MinimumDistance * MinimumDistance)) 
		{
			return false;
		}
		
		const float DotProduct = FVector::DotProduct(Rotation.Vector(), Direction.GetSafeNormal());
		const float AngleRadians = FMath::Acos(DotProduct);
		const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
    
		return AngleDegrees <= (Degrees * 0.5f);
	}

	/**
	 * Checks if multiple points are inside or on the surface of an arc.
	 * @param Points The array of points to check.
	 * @param Origin The center point of an arc.
	 * @param Rotation The base rotation of the arc.
	 * @param Degrees The angle of the arc in degrees.
	 * @param MinimumDistance The minimum distance of the arc.
	 * @param MaximumDistance The maximum distance of the arc.
	 * @return An array of boolean values indicating if each point is inside or on the surface of the FBox.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FRotator& Rotation, const float& Degrees, const float& MinimumDistance, const float& MaximumDistance)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			OutResults.Add(IsPointInsideOrOn(Origin, Rotation, Degrees, MinimumDistance, MaximumDistance, Point));
		}
		return MoveTemp(OutResults);
	}
};