// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NRectanglePickerParams.h"

/**
 * Provides various functions for generating points the plane of a rectangle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/">FNRectanglePicker</a>
 */
class NEXUSPICKER_API FNRectanglePicker
{
public:

	/**
	 * Generates a deterministic point inside or on the plane of a rectangle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params);

	/**
	 * Generates a deterministic point inside or on the plane of a rectangle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params);
	
	/**
	 * Generates a deterministic point inside or on the plane of a rectangle.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNRectanglePickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
		Tracked(OutLocations, DuplicateSeed, Params);
	}
	
	/**
	 * Generates a deterministic point inside or on the plane of a rectangle.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNRectanglePickerParams& Params);
	
	/**
	 * Checks if a point is inside or on the plane of a rectangle.
	 * Takes into account the rotation of the rectangle, if any.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the plane of the rectangle, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			if (Point.Z != Origin.Z) return false;
			if (Point.X < Origin.X - ExtentX || Point.X > Origin.X + ExtentX) return false;
			if (Point.Y < Origin.Y - ExtentY || Point.Y > Origin.Y + ExtentY) return false;
			return true;
		}

		const FVector UnrotatedPoint = Rotation.UnrotateVector(Point);
		if (UnrotatedPoint.Z != Origin.Z) return false;
		if (UnrotatedPoint.X < Origin.X - ExtentX || UnrotatedPoint.X > Origin.X + ExtentX) return false;
		if (UnrotatedPoint.Y < Origin.Y - ExtentY || UnrotatedPoint.Y > Origin.Y + ExtentY) return false;
		return true;
	}

	/**
	 * Checks if multiple points are inside or on the plane of a rectangle.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle.
	 * @param MaximumDimensions The maximum dimensions of the rectangle.
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return An array of boolean values indicating if each point is inside or on the surface of the rectangle.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
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