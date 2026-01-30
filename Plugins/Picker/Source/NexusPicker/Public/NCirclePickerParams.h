// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NCirclePickerParams.generated.h"

/**
 * Parameters for the circle picker functionality, used to define boundaries
 * for generating points within a specified circle.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNCirclePickerParams : public FNPickerParams
{
	GENERATED_BODY()

	/**
	 * The rotation of the circle plane.
	 */
	UPROPERTY(Category = "Circle", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/**
	 * The minimum radius of the circle (inner bound).
	 */
	UPROPERTY(Category = "Circle", BlueprintReadWrite)
	float MinimumRadius = 0.f;

	/**
	 * The maximum radius of the circle (outer bound).
	 */
	UPROPERTY(Category = "Circle", BlueprintReadWrite)
	float MaximumRadius = 10.f;
};
