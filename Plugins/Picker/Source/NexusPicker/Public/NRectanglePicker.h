// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NRectanglePickerParams.h"
#include "Math/NMersenneTwister.h"

/**
 * Provides various functions for generating points the plane of a rectangle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/">FNRectanglePicker</a>
 */
class NEXUSPICKER_API FNRectanglePicker
{
public:

	/**
	 * Generate random points inside or on the plane of a rectangle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 * @note Not thread-safe; all pickers share a single non-deterministic FRandomStream (FNRandom::GetNonDeterministic()). Only call from the Game-thread.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params);
	
	/**
	 * Generate random points inside or on the plane of a rectangle.
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
	 * Generate random points inside or on the plane of a rectangle.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNRectanglePickerParams& Params);
	
	/**
	 * Generate random points inside or on the plane of a rectangle using a provided Mersenne Twister.	 
	 * @param OutLocations An array to store the generated points.
	 * @param Random The Mersenne Twister to query for random.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNRectanglePickerParams& Params);
	
	/**
	 * Checks if a point is inside or on the plane of a rectangle.
	 * Takes into account the rotation of the rectangle, if any.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the plane of the rectangle, false otherwise.
	 * @note Inclusive: a point exactly on an edge returns true.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		const FMatrix RotationMatrix = FRotationMatrix::Make(Rotation);
		const FVector LocalPoint = RotationMatrix.InverseTransformPosition(Point - Origin);
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;

		return LocalPoint.X >= -ExtentX && LocalPoint.X <= ExtentX &&
			   LocalPoint.Y >= -ExtentY && LocalPoint.Y <= ExtentY;
	}

	/**
	 * Checks if a point is strictly inside the plane of a rectangle, excluding its edges.
	 * Takes into account the rotation of the rectangle, if any.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param Point The point to check.
	 * @return True if the point is strictly inside the rectangle, false if on an edge or outside.
	 */
	FORCEINLINE static bool IsPointInside(const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		const FMatrix RotationMatrix = FRotationMatrix::Make(Rotation);
		const FVector LocalPoint = RotationMatrix.InverseTransformPosition(Point - Origin);
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;

		return LocalPoint.X > -ExtentX && LocalPoint.X < ExtentX &&
			   LocalPoint.Y > -ExtentY && LocalPoint.Y < ExtentY;
	}

	/**
	 * Checks if a point is inside or on the band between two concentric rectangles.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle.
	 * @param MaximumDimensions The maximum dimensions of the rectangle.
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the rectangular band, false otherwise.
	 * @note Closed band: points on the inner OR outer edge are included; only points strictly inside MinimumDimensions (the hole) are excluded. When MinimumDimensions is zero there is no hole, so the center is included.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation, const FVector& Point)
	{
		return IsPointInsideOrOn(Origin, MaximumDimensions, Rotation, Point) && !IsPointInside(Origin, MinimumDimensions, Rotation, Point);
	}

	/**
	 * Checks if multiple points are inside or on the band between two concentric rectangles.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle.
	 * @param MaximumDimensions The maximum dimensions of the rectangle.
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return An array of boolean values indicating if each point is inside or on the rectangular band.
	 * @note Closed band: points on the inner OR outer edge are included; only points strictly inside MinimumDimensions (the hole) are excluded. When MinimumDimensions is zero there is no hole, so every point inside or on MaximumDimensions (including the center) is included.
	 */
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
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