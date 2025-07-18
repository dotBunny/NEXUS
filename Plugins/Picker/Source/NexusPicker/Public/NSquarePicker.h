// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

// TODO: Add Inner/Outer Dimension Option (#37)
// TODO: GRound ->PRojected Naming

/**
 * Provides various functions for generating points the plane of a square using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 */
class NEXUSPICKER_API FNSquarePicker
{
public:
	/**
	 * Generates a deterministic point inside or on the plane of a square.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the square.
	 * @param MinimumDimensions The minimum width and height of the square (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the square (X and Y values, outer bound).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
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
					FNRandom::Deterministic.FloatRange(Origin.X - MinimumExtentX, Origin.X - MaximumExtentX) :
					FNRandom::Deterministic.FloatRange(Origin.X + MinimumExtentX, Origin.X + MaximumExtentX),
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(Origin.X - MinimumExtentY, Origin.X - MaximumExtentY) :
					FNRandom::Deterministic.FloatRange(Origin.X + MinimumExtentY, Origin.X + MaximumExtentY),
					Origin.Z);
		}
		else
		{
			const FVector UnrotatedPosition = Origin + FVector(
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(Origin.X - MinimumExtentX, Origin.X - MaximumExtentX) :
					FNRandom::Deterministic.FloatRange(Origin.X + MinimumExtentX, Origin.X + MaximumExtentX),
				FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(Origin.X - MinimumExtentY, Origin.X - MaximumExtentY) :
					FNRandom::Deterministic.FloatRange(Origin.X + MinimumExtentY, Origin.X + MaximumExtentY),
					Origin.Z);
			OutLocation = Rotation.RotateVector(UnrotatedPosition);
		}
	}

	/**
	 * Generates a deterministic point inside or on the plane of a square.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void NextPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = FVector(
				FNRandom::Deterministic.FloatRange(Origin.X - ExtentX, Origin.X + ExtentX),
				FNRandom::Deterministic.FloatRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
				Origin.Z);
		}
		else
		{
			const FVector UnrotatedPosition = FVector(
				FNRandom::Deterministic.FloatRange(Origin.X - ExtentX, Origin.X + ExtentX),
				FNRandom::Deterministic.FloatRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
				Origin.Z);
			OutLocation = Rotation.RotateVector(UnrotatedPosition);
		}
		
	}

	/**
	 * Generates a deterministic point inside or on the plane of a square, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the square.
	 * @param MinimumDimensions The minimum width and height of the square (X and Y values, inner bound).
	 * @param MaximumDimensions The maximum width and height of the square (X and Y values, outer bound).
	 * @param Rotation The rotation of the square around its center.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator& Rotation, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}

	FORCEINLINE static void NextPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOnSimple(OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the plane of a square.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = FVector(
			FNRandom::NonDeterministic.RandRange(Origin.X - ExtentX, Origin.X + ExtentX),
			FNRandom::NonDeterministic.RandRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
				Origin.Z);
		}
		else
		{
			const FVector UnrotatedPosition = FVector(
			FNRandom::NonDeterministic.RandRange(Origin.X - ExtentX, Origin.X + ExtentX),
			FNRandom::NonDeterministic.RandRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
				Origin.Z);
			OutLocation = Rotation.RotateVector(UnrotatedPosition);
		}
		
	}

	/**
	 * Generates a random point inside or on the plane of a square, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the plane of a square using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation)
	{
		const FRandomStream RandomStream(Seed);
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = FVector(
			RandomStream.RandRange(Origin.X - ExtentX, Origin.X + ExtentX),
			RandomStream.RandRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
			Origin.Z);
		}
		else
		{
			const FVector UnrotatedPosition = FVector(
			RandomStream.RandRange(Origin.X - ExtentX, Origin.X + ExtentX),
			RandomStream.RandRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
			Origin.Z);
			OutLocation = Rotation.RotateVector(UnrotatedPosition);
		}
		
	}

	/**
	 * Generates a random point inside or on the plane of a square using a provided seed, then projects it to the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the plane of a square while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const FRandomStream RandomStream(Seed);
		const float ExtentX = Dimensions.X * 0.5f;
		const float ExtentY = Dimensions.Y * 0.5f;
		if (Rotation.IsZero())
		{
			OutLocation = FVector(
			RandomStream.RandRange(Origin.X - ExtentX, Origin.X + ExtentX),
			RandomStream.RandRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
			Origin.Z);
		}
		else
		{
			const FVector UnrotatedPosition = FVector(
			RandomStream.RandRange(Origin.X - ExtentX, Origin.X + ExtentX),
			RandomStream.RandRange(Origin.Y - ExtentY, Origin.Y + ExtentY),
			Origin.Z);
			OutLocation = Rotation.RotateVector(UnrotatedPosition);
		}
		
		Seed = RandomStream.GetCurrentSeed();
	}

	/**
	 * Generates a random point inside or on the plane of a square while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const FVector2D& Dimensions, const FRotator& Rotation = FRotator::ZeroRotator, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, Dimensions, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Checks if a point is inside or on the plane of a square.
	 * Takes into account the rotation of the square if any.
	 * 
	 * @param Origin The center point of the square.
	 * @param Dimensions The width and height of the square (X and Y values).
	 * @param Rotation The rotation of the square around its center.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the plane of the square, false otherwise.
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