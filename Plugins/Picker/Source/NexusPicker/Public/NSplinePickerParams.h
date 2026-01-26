// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "Components/SplineComponent.h"
#include "NSplinePickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNSplinePickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<USplineComponent> SplineComponent = nullptr;
};
