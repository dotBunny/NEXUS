// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerMinimal.h"
#include "NPickerParams.h"
#include "NOrientedBoxPickerParams.generated.h"


/**
 * Parameters for the box picker functionality, used to define boundaries
 * for generating points within a specified FOrientedBox.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNOrientedBoxPickerParams : public FNPickerParams
{
	GENERATED_BODY()
	
	/** The center point when attempting to generate new points. */
	UPROPERTY(Category = "OrientedBox", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	/**
	 * The minimum dimensions to use when generating a point.
	 * @note If left unset, will use simplified faster logic to generate points.
	 */
	UPROPERTY(Category = "OrientedBox", BlueprintReadWrite)
	FVector MinimumDimensions = FVector::ZeroVector;
	
	/** The maximum dimensions to use when generating a point. */
	UPROPERTY(Category = "OrientedBox", BlueprintReadWrite)
	FVector MaximumDimensions = FVector::OneVector;
	
	/** The rotation of the OrientedBox. */
	UPROPERTY(Category = "OrientedBox", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/**
	 * Set up parameters based on a provided FOrientedBox.
	 * @param OrientedBox FOrientedBox to use for initialization.
	 * @note Assumes the box axes are orthonormal (unit length).
	 */
	void InitializeFrom(const FOrientedBox& OrientedBox)
	{
		Rotation = FMatrix(OrientedBox.AxisX, OrientedBox.AxisY, OrientedBox.AxisZ, FVector::ZeroVector).Rotator();
		Origin = OrientedBox.Center;
		// An FOrientedBox has no inner hole, so clear any stale shell config from a reused struct
		MinimumDimensions = FVector::ZeroVector;
		// Dimensions are local-space (Rotation is applied on top), so use the extents directly
		MaximumDimensions = FVector(OrientedBox.ExtentX, OrientedBox.ExtentY, OrientedBox.ExtentZ) * 2.0f;
	}
	
	/** @return An origin-centered, axis-aligned box sized from MinimumDimensions, or an empty box when MinimumDimensions is zero. */
	FBox GetMinimumAlignedBox() const
	{
		if (MinimumDimensions.IsZero())
		{
			return FBox(ForceInit);
		}
		const FVector Half = MinimumDimensions * 0.5f;
		return FBox(- Half,  Half);
	}
	
	/** @return An origin-centered, axis-aligned box sized from MaximumDimensions, or an empty box when MaximumDimensions is zero. */
	FBox GetMaximumAlignedBox() const
	{
		if (MaximumDimensions.IsZero())
		{
			return FBox(ForceInit);
		}
		const FVector Half = MaximumDimensions * 0.5f;
		return FBox( -Half,  Half);
	}
	
	/**
	 * Builds the axis-aligned regions valid for point generation, in box-local space (before Rotation is applied).
	 * @return The maximum box alone when no valid minimum is set (or the minimum exceeds the maximum on any axis); otherwise the six side regions of the shell between the minimum and maximum boxes.
	 * @note Allocates and derives the decomposition on every call; callers that invoke a picker repeatedly with
	 * stable params pay this cost each time. Cheap relative to per-point work, but do not call it inside a hot loop.
	 */
	[[nodiscard]] TArray<FBox> GetValidBoxes() const
	{
		const FBox MinimumBox = GetMinimumAlignedBox();
		const FBox MaximumBox = GetMaximumAlignedBox();
		
		TArray<FBox> Boxes;
		if (MinimumBox.IsValid == 0)
		{
			Boxes.Add(MaximumBox);
		}
		else if (MinimumDimensions.X > MaximumDimensions.X || MinimumDimensions.Y > MaximumDimensions.Y || MinimumDimensions.Z > MaximumDimensions.Z)
		{
			UE_LOG(LogNexusPicker, Warning, TEXT("The MinimumDimensions exceeds the MaximumDimensions on at least one axis, using MaximumDimensions instead."));
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
		return Boxes;
	}
};
