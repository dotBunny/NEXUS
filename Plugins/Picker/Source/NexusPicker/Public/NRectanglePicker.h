// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

/**
 * Provides various functions for generating points the plane of a rectangle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/">FNRectanglePicker</a>
 */
class NEXUSPICKER_API FNRectanglePicker
{
public:

	// NEXT POINT
	
	/**
	 * Generates a deterministic point inside or on the plane of a rectangle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float MaximumExtentX = MaximumDimensions.X * 0.5f;
		const float MaximumExtentY = MaximumDimensions.Y * 0.5f;
		const float MinimumExtentX = MinimumDimensions.X * 0.5f;
		const float MinimumExtentY = MinimumDimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtentX, -MaximumExtentX) :
					FNRandom::Deterministic.FloatRange(MinimumExtentX, MaximumExtentX),
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtentY, -MaximumExtentY) :
					FNRandom::Deterministic.FloatRange(MinimumExtentY, MaximumExtentY),
					0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtentX, -MaximumExtentX) :
					FNRandom::Deterministic.FloatRange(MinimumExtentX, MaximumExtentX),
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtentY, -MaximumExtentY) :
					FNRandom::Deterministic.FloatRange(MinimumExtentY, MaximumExtentY),
					0.f));
		}
		
		N_IMPLEMENT_VLOG_RECTANGLE()
	}

	/**
	 * Generates a deterministic point inside or on the plane of a rectangle, then projects it to the world.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION()
	}

	/**
	 * Generates a deterministic point inside or on the plane of a rectangle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void NextPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
				FNRandom::Deterministic.FloatRange(-ExtentX, ExtentX),
				FNRandom::Deterministic.FloatRange(-ExtentY, ExtentY),
				0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
				FNRandom::Deterministic.FloatRange(-ExtentX, ExtentX),
				FNRandom::Deterministic.FloatRange(-ExtentY, ExtentY),
				0.f));
		}
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE()
	}

	/**
	 * Generates a deterministic point inside or on the plane of a rectangle, then projects it to the world.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOnSimple(OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION()
	}

	// RANDOM POINT
	
	/**
	 * Generates a random point inside or on the plane of a rectangle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float MaximumExtentX = MaximumDimensions.X * 0.5f;
		const float MaximumExtentY = MaximumDimensions.Y * 0.5f;
		const float MinimumExtentX = MinimumDimensions.X * 0.5f;
		const float MinimumExtentY = MinimumDimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
				FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtentX, -MaximumExtentX) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtentX, MaximumExtentX),
					FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtentY, -MaximumExtentY) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtentY, MaximumExtentY),
					0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
				FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtentX, -MaximumExtentX) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtentX, MaximumExtentX),
					FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtentY, -MaximumExtentY) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtentY, MaximumExtentY),
					0.f));
		}

		N_IMPLEMENT_VLOG_RECTANGLE()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle, then projects it to the world.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
			FNRandom::NonDeterministic.FRandRange(-ExtentX, ExtentX),
			FNRandom::NonDeterministic.FRandRange(-ExtentY, ExtentY),
				0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
			FNRandom::NonDeterministic.FRandRange(-ExtentX, ExtentX),
			FNRandom::NonDeterministic.FRandRange(-ExtentY, ExtentY),
				0.f));
		}
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle, then projects it to the world.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOnSimple(OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION()
	}

	// RANDOM ONE-SHOT POINT
	
	/**
	 * Generates a random point inside or on the plane of a rectangle using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation)
	{
		const FRandomStream RandomStream(Seed);
		
		const float MaximumExtentX = MaximumDimensions.X * 0.5f;
		const float MaximumExtentY = MaximumDimensions.Y * 0.5f;
		const float MinimumExtentX = MinimumDimensions.X * 0.5f;
		const float MinimumExtentY = MinimumDimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
			RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentX, -MaximumExtentX) :
					RandomStream.FRandRange(MinimumExtentX, MaximumExtentX),
					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentY, -MaximumExtentY) :
					RandomStream.FRandRange(MinimumExtentY, MaximumExtentY),
					0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
			RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentX, -MaximumExtentX) :
					RandomStream.FRandRange(MinimumExtentX, MaximumExtentX),
					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentY, -MaximumExtentY) :
					RandomStream.FRandRange(MinimumExtentY, MaximumExtentY),
					0.f));
		}
		N_IMPLEMENT_VLOG_RECTANGLE()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle using a provided seed, then projects it to the world.
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnSimple(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation)
	{
		const FRandomStream RandomStream(Seed);
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
			RandomStream.FRandRange(-ExtentX, ExtentX),
			RandomStream.FRandRange(-ExtentY, ExtentY),
			0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
			RandomStream.FRandRange(-ExtentX, ExtentX),
			RandomStream.FRandRange(-ExtentY, ExtentY),
			0.f));
		}

		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle using a provided seed, then projects it to the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnSimpleProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOnSimple(Seed, OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION()
	}

	// RANDOM TRACKED POINT

	/**
	 * Generates a random point inside or on the plane of a rectangle while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const FRandomStream RandomStream(Seed);
		const float MaximumExtentX = MaximumDimensions.X * 0.5f;
		const float MaximumExtentY = MaximumDimensions.Y * 0.5f;
		const float MinimumExtentX = MinimumDimensions.X * 0.5f;
		const float MinimumExtentY = MinimumDimensions.Y * 0.5f;

		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
			RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentX, -MaximumExtentX) :
					RandomStream.FRandRange(MinimumExtentX, MaximumExtentX),
					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentY, -MaximumExtentY) :
					RandomStream.FRandRange(MinimumExtentY, MaximumExtentY),
					0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
			RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentX, -MaximumExtentX) :
					RandomStream.FRandRange(MinimumExtentX, MaximumExtentX),
					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
					RandomStream.FRandRange(-MinimumExtentY, -MaximumExtentY) :
					RandomStream.FRandRange(MinimumExtentY, MaximumExtentY),
					0.f));
		}
		Seed = RandomStream.GetCurrentSeed();
		N_IMPLEMENT_VLOG_RECTANGLE()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum width and height of the rectangle (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the rectangle (X and Y values, outer bound).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnSimple(int32& Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const FRandomStream RandomStream(Seed);
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = Origin + FVector(
			RandomStream.FRandRange(-ExtentX, ExtentX),
			RandomStream.FRandRange(-ExtentY, ExtentY),
			0.f);
		}
		else
		{
			OutLocation = Origin + Rotation.RotateVector(FVector(
			RandomStream.FRandRange(-ExtentX, ExtentX),
			RandomStream.FRandRange(-ExtentY, ExtentY),
			0.f));
		}
		
		Seed = RandomStream.GetCurrentSeed();
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE()
	}

	/**
	 * Generates a random point inside or on the plane of a rectangle while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The width and height of the rectangle (X and Y values).
	 * @param Rotation The rotation of the rectangle around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnSimpleProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOnSimple(Seed, OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION()
	}

	// ASSERT
	
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
};