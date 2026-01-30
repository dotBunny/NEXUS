// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NBoxPickerParams.generated.h"


/**
 * Parameters for the box picker functionality, used to define boundaries
 * for generating points within a specified FBox.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNBoxPickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	/**
	 * The minimum dimensions to use when generating a point.
	 * @note If left unset, will use simplified faster logic to generate points.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FBox MinimumDimensions = FBox(ForceInit);
	
	/**
	 * The maximum dimensions to use when generating a point.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FBox MaximumDimensions = FBox(ForceInit);
};
