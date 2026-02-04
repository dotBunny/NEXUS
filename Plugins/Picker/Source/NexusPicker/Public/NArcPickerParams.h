// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NArcPickerParams.generated.h"


/**
 * Parameters for the arc picker functionality, used to define boundaries
 * for generating points within an arc.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNArcPickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	/**
	 * The center point when attempting to generate new points.
	 */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;

	/**
	 * The base rotation used when trying to determine the arc angle.
	 */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/**
	 * The degrees of the arc.
	 */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	float Degrees = 90.f;
	
	/**
	 * The minimum distance to start finding points.
	 */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	float MinimumDistance = 0.f;

	/**
	 * The maximum distance to find points.
	 */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	float MaximumDistance = 500.f;
};
