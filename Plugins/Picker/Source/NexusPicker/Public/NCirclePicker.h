// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

/**
 * Methods for selecting points related to a circle.
 */
class NEXUSPICKER_API FNCirclePicker
{
public:
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const float PointTheta = FNRandom::Deterministic.Float() * 2.0f * PI;
		const float PointRadius = FNRandom::Deterministic.FloatRange(MinimumRadius, MaximumRadius);

		OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
				Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
	}
	FORCEINLINE static void NextGroundedPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
			N_VARIABLES_PICKER_GROUNDED())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const float PointTheta = FNRandom::NonDeterministic.FRand() * 2.0f * PI;
		const float PointRadius = FNRandom::NonDeterministic.FRandRange(MinimumRadius, MaximumRadius);

		OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
				Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
	}
	FORCEINLINE static void RandomGroundedPointInsideOrOn(FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		const float PointTheta = RandomStream.FRand() * 2.0f * PI;
		const float PointRadius = RandomStream.FRandRange(MinimumRadius, MaximumRadius);

		OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
				Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
	}
	FORCEINLINE static void RandomOneShotGroundedPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
	
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius)
	{
		const FRandomStream RandomStream(Seed);
		const float PointTheta = RandomStream.FRand() * 2.0f * PI;
		const float PointRadius = RandomStream.FRandRange(MinimumRadius, MaximumRadius);

		OutLocation = FVector(Origin.X + (PointRadius * FMath::Cos(PointTheta)),
				Origin.Y + (PointRadius * FMath::Sin(PointTheta)), Origin.Z);
		Seed = RandomStream.GetCurrentSeed();
	}
	FORCEINLINE static void RandomTrackedGroundedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin,
		const float MinimumRadius, const float MaximumRadius,
		N_VARIABLES_PICKER_GROUNDED())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumRadius, MaximumRadius);
		N_IMPLEMENT_PICKER_GROUNDED()
	}
};
