// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

/**
 * Provides various functions for generating points the plane of a circle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 */
class NEXUSPICKER_API FNCirclePicker
{
public:

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

	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
			N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
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

	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
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

	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
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

	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius, const FRotator& Rotation = FRotator::ZeroRotator,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		N_IMPLEMENT_PICKER_PROJECTION()
	}
	
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
