// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "Components/SplineComponent.h"
#include "NSplinePickerParams.generated.h"

/**
 * Parameters for the spline picker functionality, used to define boundaries for generating points within a 
 * specified spline.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNSplinePickerParams : public FNPickerParams
{
	GENERATED_BODY()

	/**
	 * The spline component to generate points on.
	 */
	UPROPERTY(Category = "Spline", BlueprintReadWrite)
	TObjectPtr<USplineComponent> SplineComponent = nullptr;
};
