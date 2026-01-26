// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NRectanglePickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNRectanglePickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	
	UPROPERTY()
	FVector2D MinimumDimensions = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D MaximumDimensions = FVector2D(1.f,1.f);
};
