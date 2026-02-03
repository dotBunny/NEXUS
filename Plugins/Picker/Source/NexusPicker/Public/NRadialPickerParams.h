// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NRadialPickerParams.generated.h"


/**
 * Parameters for the radial picker functionality, used to define boundaries
 * for generating points within the radial selection.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNRadialPickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	/**
	 * The center point when attempting to generate new points.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;

	/**
	 * The base rotation used when trying to determine the radial angles
	 */
	UPROPERTY(Category = "Radial", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/**
	 * The minimum distance to start finding points.
	 */
	UPROPERTY(Category = "Radial", BlueprintReadWrite)
	float MinimumDistance = 0.f;

	/**
	 * The maximum distance to find points.
	 */
	UPROPERTY(Category = "Radial", BlueprintReadWrite)
	float MaximumDistance = 500.f;

	/**
	 * The start of the angle/area for point selection.
	 */
	UPROPERTY(Category = "Radial", BlueprintReadWrite)
	float MinimumAngle = -45.f;

	/**
	 * The end of the angle/area for point selection.
	 */
	UPROPERTY(Category = "Radial", BlueprintReadWrite)
	float MaximumAngle = 45.f;
};
