// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerMinimal.h"
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
	 * The center point when attempting to generate new points.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	/**
	 * The minimum dimensions to use when generating a point.
	 * @note If left unset, will use simplified faster logic to generate points.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FBox MinimumBox = FBox(ForceInit);
	
	/**
	 * The maximum dimensions to use when generating a point.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FBox MaximumBox = FBox(ForceInit);

	/**
	 * Gets an array of FBox that can be selected from based on the min/max.
	 * @return An array of valid FBox.
	 */
	TArray<FBox> GetValidBoxes() const
	{
		TArray<FBox> Boxes;
		if (MinimumBox.IsValid == 0)//|| )
		{
			Boxes.Add(MaximumBox);
		}
		else if (!MaximumBox.IsInside(MinimumBox.Min) || !MaximumBox.IsInside(MinimumBox.Max))
		{
			UE_LOG(LogNexusPicker, Warning, TEXT("The minimum box is not fully enclosed by the MaximumBox, this is not supported and will default to using the MaximumBox."));
			Boxes.Add(MaximumBox);
		}
		else
		{
			// Reserve the number of possible sides just because
			Boxes.Reserve(6);
			
			// Left
			Boxes.Add(FBox(
				FVector(MaximumBox.Min.X, MaximumBox.Min.Y, MaximumBox.Min.Z),
				FVector(MinimumBox.Min.X, MaximumBox.Max.Y, MaximumBox.Max.Z)
			));
			
			// Right
			Boxes.Add(FBox(
			FVector(MinimumBox.Max.X, MaximumBox.Min.Y, MaximumBox.Min.Z),
			FVector(MaximumBox.Max.X, MaximumBox.Max.Y, MaximumBox.Max.Z)));
		
			// Front
			Boxes.Add(FBox(
				FVector(MinimumBox.Min.X, MaximumBox.Min.Y, MaximumBox.Min.Z),
				FVector(MinimumBox.Max.X, MinimumBox.Min.Y, MinimumBox.Max.Z)
			));
		
			// Back
			Boxes.Add(FBox(
				FVector(MinimumBox.Min.X, MinimumBox.Max.Y, MinimumBox.Min.Z),
				FVector(MinimumBox.Max.X, MaximumBox.Max.Y, MaximumBox.Max.Z)
			));
			
			// Bottom
			Boxes.Add(FBox(
				FVector(MinimumBox.Min.X, MinimumBox.Min.Y, MaximumBox.Min.Z),
				FVector(MinimumBox.Max.X, MaximumBox.Max.Y, MinimumBox.Min.Z)
			));
			
			// Top
			Boxes.Add(FBox(
				FVector(MinimumBox.Min.X, MaximumBox.Min.Y, MinimumBox.Max.Z),
				FVector(MinimumBox.Max.X, MinimumBox.Max.Y, MaximumBox.Max.Z)
			));
		}
		return MoveTemp(Boxes);
	}
};
