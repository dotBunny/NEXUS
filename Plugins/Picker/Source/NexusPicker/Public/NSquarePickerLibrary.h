// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSquarePicker.h"
#include "NSquarePickerLibrary.generated.h"

/**
 * Blueprint function library providing access to square picker functionality.
 * Exposes FNSquarePicker methods for use in Blueprints with a simplified interface.
 */
UCLASS()
class NEXUSPICKER_API UNSquarePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable, DisplayName = "Square: Next Point (Inside Or On)", Category = "NEXUS|Picker|Square")
	static FVector NextPointInsideOrOn(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNSquarePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}
};