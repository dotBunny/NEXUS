// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NRectanglePickerParams.generated.h"

/**
 * Parameters for the rectangle picker functionality, used to define boundaries for generating points within a 
 * specified rectangle.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNRectanglePickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	/**
	 * The center point when attempting to generate new points.
	 */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	
	/**
	 * The inner dimensions of the rectangle (X = width, Y = height).
	 */	
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FVector2D MinimumDimensions = FVector2D::ZeroVector;

	/**
	 * The outer dimensions of the rectangle (X = width, Y = height).
	 */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FVector2D MaximumDimensions = FVector2D(1.f,1.f);
	
	/**
	 * The rotation of the rectangle plane.
	 */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
};
