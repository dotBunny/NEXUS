// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerMinimal.h"
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
	
	/** The center point when attempting to generate new points. */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	
	/** The inner dimensions of the rectangle (X = width, Y = height). */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FVector2D MinimumDimensions = FVector2D::ZeroVector;

	/** The outer dimensions of the rectangle (X = width, Y = height). */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FVector2D MaximumDimensions = FVector2D(1.f,1.f);
	
	/** The rotation of the rectangle plane. */
	UPROPERTY(Category = "Rectangle", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
	
	/**
	 * Gets the ranges which can be selected from.
	 * @note This will appropriately cut out portions of available space when the MinimumDimensions are not fully enclosed by the MaximumDimensions.
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
		else if (MinimumDimensions.X > MaximumDimensions.X || MinimumDimensions.Y > MaximumDimensions.Y)
		{
			UE_LOG(LogNexusPicker, Warning, TEXT("The MinimumDimensions exceeds the MaximumDimensions on at least one axis, using MaximumDimensions instead."));
			ValidRanges.Add( FVector4(
				-HalfMaxDimensions.X, -HalfMaxDimensions.Y,  // Min
				HalfMaxDimensions.X, HalfMaxDimensions.Y)); // Max
		}
		else if (MinimumDimensions.X >= MaximumDimensions.X && MinimumDimensions.Y >= MaximumDimensions.Y)
		{
			// Reaching here means neither axis exceeds (both Min <= Max), so this is the Min == Max case on
			// both axes: the inner hole fully covers the rectangle and leaves zero annulus area. The strip/corner
			// builder below uses strict '<' comparisons, so it would return an EMPTY ValidRanges, which the pickers
			// then index out-of-bounds (ValidRanges[Random(0, -1)]). Fall back to the full rectangle so callers
			// always receive a usable range instead of crashing.
			UE_LOG(LogNexusPicker, Warning, TEXT("The MinimumDimensions matches the MaximumDimensions on both axes (no valid area); using MaximumDimensions instead."));
			ValidRanges.Add( FVector4(
				-HalfMaxDimensions.X, -HalfMaxDimensions.Y,  // Min
				HalfMaxDimensions.X, HalfMaxDimensions.Y)); // Max
		}
		else
		{
			const FVector2D HalfMinDimensions = MinimumDimensions * 0.5f;

			// Inner-hole extents clamped to the outer rectangle. Ranges below are packed as
			// (min_x, min_y, max_x, max_y) so consumers can call FloatValue(min, max) without
			// risking inverted args (std::uniform_real_distribution requires a <= b).
			const float HoleMinX = FMath::Max(-HalfMinDimensions.X, -HalfMaxDimensions.X);
			const float HoleMaxX = FMath::Min(HalfMinDimensions.X, HalfMaxDimensions.X);
			const float HoleMinY = FMath::Max(-HalfMinDimensions.Y, -HalfMaxDimensions.Y);
			const float HoleMaxY = FMath::Min(HalfMinDimensions.Y, HalfMaxDimensions.Y);

			// Reserve the number of possible sides just because
			ValidRanges.Reserve(8);

			// We can do some really simple checks for the absolute vertical spots
			if (HalfMinDimensions.Y < HalfMaxDimensions.Y)
			{
				// North | top strip
				ValidRanges.Add(FVector4(HoleMinX, HoleMaxY, HoleMaxX, HalfMaxDimensions.Y));

				// South | bottom strip
				ValidRanges.Add(FVector4(HoleMinX, -HalfMaxDimensions.Y, HoleMaxX, HoleMinY));
			}

			// We can do some really simple checks for the absolute horizontal spots
			if (HalfMinDimensions.X < HalfMaxDimensions.X)
			{
				// East | right strip
				ValidRanges.Add(FVector4(HoleMaxX, HoleMinY, HalfMaxDimensions.X, HoleMaxY));

				// West | left strip
				ValidRanges.Add(FVector4(-HalfMaxDimensions.X, HoleMinY, HoleMinX, HoleMaxY));
			}

			// If we're inside and do not overlap we can make our corners
			if (HalfMinDimensions.Y < HalfMaxDimensions.Y && HalfMinDimensions.X < HalfMaxDimensions.X)
			{
				// North East | top right corner
				ValidRanges.Add(FVector4(HoleMaxX, HoleMaxY, HalfMaxDimensions.X, HalfMaxDimensions.Y));

				// South East | bottom right corner
				ValidRanges.Add(FVector4(HoleMaxX, -HalfMaxDimensions.Y, HalfMaxDimensions.X, HoleMinY));

				// South West | bottom left corner
				ValidRanges.Add(FVector4(-HalfMaxDimensions.X, -HalfMaxDimensions.Y, HoleMinX, HoleMinY));

				// North West | top left corner
				ValidRanges.Add(FVector4(-HalfMaxDimensions.X, HoleMaxY, HoleMinX, HalfMaxDimensions.Y));
			}
		}
		return ValidRanges;
	}
};
