// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCirclePickerParams.h"

/**
 * Provides various functions for generating points in the plane of a circle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/circle/">FNCirclePicker</a>
 * @note There are no simple variants to these methods as it wouldn't change the computation.
 */
class NEXUSPICKER_API FNCirclePicker
{
public:

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params);

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params);
	

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNCirclePickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
		Tracked(OutLocations, DuplicateSeed, Params);
	}
	
	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNCirclePickerParams& Params);
	
	/**
	 * Checks if a point is inside or on the perimeter of a circle.	 
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation The rotation of the circle plane.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the perimeter of the circle, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation, const FVector& Point)
	{
		const FVector CircleNormal = Rotation.RotateVector(FVector::UpVector);
		const FVector PointNormal = Point - Origin;
		const float DistanceToPlane = FVector::DotProduct(PointNormal, CircleNormal);
		const FVector ProjectedPoint = Point - (CircleNormal * DistanceToPlane);
		const float DistanceSquared = FVector::DistSquared(ProjectedPoint, Origin);
		return DistanceSquared >= (MinimumRadius * MinimumRadius) && DistanceSquared <= ( MaximumRadius * MaximumRadius);
	}

	/**
	 * Checks if multiple points are inside or on the perimeter of a circle.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the FBox.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation The rotation of the circle plane.
	 * @return An array of boolean values indicating if each point is inside or on the perimeter of a circle.
	 */	
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			OutResults.Add(IsPointInsideOrOn(Origin, MinimumRadius, MaximumRadius, Rotation, Point));
		}
		return MoveTemp(OutResults);
	}
};