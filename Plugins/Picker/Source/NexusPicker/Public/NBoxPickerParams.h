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
	FBox MinimumDimensions = FBox(ForceInit);
	
	/**
	 * The maximum dimensions to use when generating a point.
	 */
	UPROPERTY(Category = "Box", BlueprintReadWrite)
	FBox MaximumDimensions = FBox(ForceInit);
	
	TArray<FBox> GetRegions() const
	{
		TArray<FBox> ValidRegions;
		ValidRegions.Reserve(6);
		if (!MaximumDimensions.IsInside(MinimumDimensions.Min) || !MaximumDimensions.IsInside(MinimumDimensions.Max))
		{
			UE_LOG(LogNexusPicker, Log, TEXT("Invalid minimum dimensions (%s) detected. Using MaximumDimensions(%s) as region."), *MinimumDimensions.ToCompactString(), *MaximumDimensions.ToCompactString());
			ValidRegions.Add(MaximumDimensions);
		}
		else
		{
			if (MaximumDimensions.Min.X < MinimumDimensions.Min.X) // LEFT
			{
				ValidRegions.Add(FBox(
					FVector(MaximumDimensions.Min.X, MaximumDimensions.Min.Y, MaximumDimensions.Min.Z),
					FVector(MinimumDimensions.Min.X, MaximumDimensions.Max.Y, MaximumDimensions.Max.Z)
				));
			}
			if (MaximumDimensions.Max.X > MinimumDimensions.Max.X) // RIGHT
			{
				ValidRegions.Add(FBox(
					FVector(MinimumDimensions.Max.X, MaximumDimensions.Min.Y, MaximumDimensions.Min.Z),
					FVector(MaximumDimensions.Max.X, MaximumDimensions.Max.Y, MaximumDimensions.Max.Z)
				));
			}
			if (MaximumDimensions.Min.Y < MinimumDimensions.Min.Y) // FRONT
			{
				ValidRegions.Add(FBox(
					FVector(MinimumDimensions.Min.X, MaximumDimensions.Min.Y, MaximumDimensions.Min.Z),
					FVector(MinimumDimensions.Max.X, MinimumDimensions.Min.Y, MinimumDimensions.Max.Z)
				));
			}
			if (MaximumDimensions.Max.Y > MinimumDimensions.Max.Y) // BACK
			{
				ValidRegions.Add(FBox(
					FVector(MinimumDimensions.Min.X, MinimumDimensions.Max.Y, MinimumDimensions.Min.Z),
					FVector(MinimumDimensions.Max.X, MaximumDimensions.Max.Y, MaximumDimensions.Max.Z)
				));
			}
			if (MaximumDimensions.Min.Z < MinimumDimensions.Min.Z) // BOTTOM
			{
				ValidRegions.Add(FBox(
					FVector(MinimumDimensions.Min.X, MinimumDimensions.Min.Y, MaximumDimensions.Min.Z),
					FVector(MinimumDimensions.Max.X, MinimumDimensions.Max.Y, MinimumDimensions.Min.Z)
				));
			}
			if (MaximumDimensions.Max.Z > MinimumDimensions.Max.Z) // TOP
			{
				ValidRegions.Add(FBox(
					FVector(MinimumDimensions.Min.X, MinimumDimensions.Min.Y, MinimumDimensions.Max.Z),
					FVector(MinimumDimensions.Max.X, MinimumDimensions.Max.Y, MaximumDimensions.Max.Z)
				));
			}
		}
		return MoveTemp(ValidRegions);
	}
};
