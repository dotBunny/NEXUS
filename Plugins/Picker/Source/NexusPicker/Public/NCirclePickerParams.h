// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NCirclePickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNCirclePickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	
	UPROPERTY()
	float MinimumRadius = -1.f;
	
	UPROPERTY()
	float MaximumRadius = 10.f;
};
