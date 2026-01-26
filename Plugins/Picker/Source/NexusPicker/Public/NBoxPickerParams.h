// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NBoxPickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNBoxPickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	UPROPERTY()
	FBox MinimumDimensions;
	
	UPROPERTY()
	FBox MaximumDimensions;
};
