// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

/**
 * Provides various functions for generating points in the plane of a circle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 */
class NEXUSPICKER_API FNCirclePicker
{
public:
	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 */
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float PointTheta = FNRandom::Deterministic.Float() * 2.0f * PI;
		const float PointRadius = FNRandom::Deterministic.FloatRange(MinimumRadius, MaximumRadius);

		if (Rotation.IsZero())
		{
			OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
					Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
		}
		else
		{
			OutLocation = Rotation.RotateVector(FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
					Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z));
		}
	}

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
			N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the perimeter of a circle.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const float PointTheta = FNRandom::NonDeterministic.FRand() * 2.0f * PI;
		const float PointRadius = FNRandom::NonDeterministic.FRandRange(MinimumRadius, MaximumRadius);

		if (Rotation.IsZero())
		{
			OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
					Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
		}
		else
		{
			OutLocation = Rotation.RotateVector(FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
					Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z));
		}
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle, then projects it to the world.
	 * 
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the perimeter of a circle using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const FRandomStream RandomStream(Seed);
		const float PointTheta = RandomStream.FRand() * 2.0f * PI;
		const float PointRadius = RandomStream.FRandRange(MinimumRadius, MaximumRadius);

		if (Rotation.IsZero())
		{
			OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
				Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
		}
		else
		{
			OutLocation = Rotation.RotateVector(FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
				Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z));
		}
		
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle using a provided seed, then projects it to the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Generates a random point inside or on the perimeter of a circle while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		const FRandomStream RandomStream(Seed);
		const float PointTheta = RandomStream.FRand() * 2.0f * PI;
		const float PointRadius = RandomStream.FRandRange(MinimumRadius, MaximumRadius);

		if (Rotation.IsZero())
		{
			OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
		Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
		}
		else
		{
			OutLocation = Rotation.RotateVector(FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
		Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z));
		}

		Seed = RandomStream.GetCurrentSeed();
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and projected point location.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
	/**
	 * Checks if a point is inside or on the perimeter of a circle.
	 * 
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation The rotation of the circle plane.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the perimeter of the circle, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation, const FVector& Point)
	{
		if (Rotation.IsZero())
		{
			if (Point.Z != Origin.Z) return false;
			
			const float DistanceSquared = FMath::Square(Point.X - Origin.X) + FMath::Square(Point.Y - Origin.Y);
			return	DistanceSquared >= FMath::Square(MinimumRadius) &&
					DistanceSquared <= FMath::Square(MaximumRadius);
		}

		const FVector UnrotatedPoint = Rotation.UnrotateVector(Point);
		if (UnrotatedPoint.Z != Origin.Z) return false;

		const float DistanceSquared = FMath::Square(UnrotatedPoint.X - Origin.X) + FMath::Square(UnrotatedPoint.Y - Origin.Y);
		return	DistanceSquared >= FMath::Square(MinimumRadius) &&
				DistanceSquared <= FMath::Square(MaximumRadius);
		
	}
};