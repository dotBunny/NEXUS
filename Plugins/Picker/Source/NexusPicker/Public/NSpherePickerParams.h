// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NSpherePickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNSpherePickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	UPROPERTY()
	float MinimumRadius = 0.f;
	
	UPROPERTY()
	float MaximumRadius = 10.f;
};
