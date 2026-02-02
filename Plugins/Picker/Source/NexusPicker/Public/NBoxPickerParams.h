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
	 * 
	 * @return An array of FBox regions that can be used respecting the min/max FBox.
	 */
	TArray<FBox> GetRegions() const
	{
		TArray<FBox> ValidRegions;
		if (MinimumBox.IsValid == 0 || !MaximumBox.IsInside(MinimumBox.Min) || !MaximumBox.IsInside(MinimumBox.Max))
		{
			UE_LOG(LogNexusPicker, Log, TEXT("Invalid minimum dimensions (%s) detected. Using MaximumDimensions(%s) as region."), *MinimumBox.ToCompactString(), *MaximumBox.ToCompactString());
			ValidRegions.Add(MaximumBox);
		}
		else
		{
			// Reserve the number of possible sides just because
			ValidRegions.Reserve(6);
			
			// Left Region
			if (MaximumBox.Min.X < MinimumBox.Min.X)
			{
				ValidRegions.Add(FBox(
					FVector(MaximumBox.Min.X, MaximumBox.Min.Y, MaximumBox.Min.Z),
					FVector(MinimumBox.Min.X, MaximumBox.Max.Y, MaximumBox.Max.Z)
				));
			}
			// Right Region
			if (MaximumBox.Max.X > MinimumBox.Max.X)
			{
				ValidRegions.Add(FBox(
					FVector(MinimumBox.Max.X, MaximumBox.Min.Y, MaximumBox.Min.Z),
					FVector(MaximumBox.Max.X, MaximumBox.Max.Y, MaximumBox.Max.Z)
				));
			}
			// Front Region
			if (MaximumBox.Min.Y < MinimumBox.Min.Y)
			{
				ValidRegions.Add(FBox(
					FVector(MinimumBox.Min.X, MaximumBox.Min.Y, MaximumBox.Min.Z),
					FVector(MinimumBox.Max.X, MinimumBox.Min.Y, MinimumBox.Max.Z)
				));
			}
			// Back Region
			if (MaximumBox.Max.Y > MinimumBox.Max.Y)
			{
				ValidRegions.Add(FBox(
					FVector(MinimumBox.Min.X, MinimumBox.Max.Y, MinimumBox.Min.Z),
					FVector(MinimumBox.Max.X, MaximumBox.Max.Y, MaximumBox.Max.Z)
				));
			}
			// Bottom Region
			if (MaximumBox.Min.Z < MinimumBox.Min.Z)
			{
				ValidRegions.Add(FBox(
					FVector(MinimumBox.Min.X, MinimumBox.Min.Y, MaximumBox.Min.Z),
					FVector(MinimumBox.Max.X, MinimumBox.Max.Y, MinimumBox.Min.Z)
				));
			}
			// Top Region
			if (MaximumBox.Max.Z > MinimumBox.Max.Z)
			{
				ValidRegions.Add(FBox(
					FVector(MinimumBox.Min.X, MinimumBox.Min.Y, MinimumBox.Max.Z),
					FVector(MinimumBox.Max.X, MinimumBox.Max.Y, MaximumBox.Max.Z)
				));
			}
		}
		return MoveTemp(ValidRegions);
	}
};
