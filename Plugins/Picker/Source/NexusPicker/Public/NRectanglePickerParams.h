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
	
	/**
	 * Gets the ranges which can be selected from.
	 * @return An array of packed Min/Max coords representing the possible area to select from based on parameters.
	 */
	TArray<FVector4> GetValidRanges() const
	{
		// Going to use an FVector4 to pack in our Min, Max
		// X/Y = Min
		// Z/W = Max
		TArray<FVector4> ValidRanges;
		const FVector2D HalfMaxDimensions = MaximumDimensions * 0.5f;
		
		if (MinimumDimensions.IsZero())
		{
			ValidRanges.Add( FVector4(
				-HalfMaxDimensions.X, -HalfMaxDimensions.Y,  // Min
				HalfMaxDimensions.X, HalfMaxDimensions.Y)); // Max
		}
		else
		{
			const FVector2D HalfMinDimensions = MinimumDimensions * 0.5f;
			
			// Reserve the number of possible sides just because
			ValidRanges.Reserve(4);
			
			// Top
			ValidRanges.Add( FVector4(-HalfMaxDimensions.X,HalfMaxDimensions.Y, 
				HalfMinDimensions.X, HalfMinDimensions.Y));
			
			// Right
			ValidRanges.Add( FVector4(HalfMinDimensions.X,HalfMaxDimensions.Y, 
				HalfMaxDimensions.X, -HalfMinDimensions.Y));
			
			// Bottom
			ValidRanges.Add( FVector4(-HalfMinDimensions.X,-HalfMinDimensions.Y, 
				HalfMaxDimensions.X,-HalfMaxDimensions.Y));
			
			// Left
			ValidRanges.Add( FVector4(-HalfMaxDimensions.X,HalfMinDimensions.Y, 
				-HalfMinDimensions.X,HalfMaxDimensions.Y));
			
		}
		return MoveTemp(ValidRanges);
	}
};
