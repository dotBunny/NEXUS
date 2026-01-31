// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NSpherePickerParams.generated.h"

/**
 * Parameters for the sphere picker functionality, used to define boundaries for generating points within a 
 * specified sphere.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNSpherePickerParams : public FNPickerParams
{
	GENERATED_BODY()

	/**
	 * The minimum radius of the sphere (inner bound).
	 */
	UPROPERTY(Category = "Sphere", BlueprintReadWrite)
	float MinimumRadius = 0.f;

	/**
	 * The maximum radius of the sphere (outer bound).
	 */
	UPROPERTY(Category = "Sphere", BlueprintReadWrite)
	float MaximumRadius = 10.f;
};
