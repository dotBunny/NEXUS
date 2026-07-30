// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.h"
#include "NArcPickerParams.generated.h"


/**
 * Parameters for the arc picker functionality, used to define boundaries
 * for generating points within an arc.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNArcPickerParams : public FNPickerParams
{
	GENERATED_BODY()

	/** The center point when attempting to generate new points. */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;

	/** The base rotation used when trying to determine the arc angle. */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** The degrees of the arc. */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	float Degrees = 90.f;

	/**
	 * The minimum distance from Origin at which points may be generated.
	 * @note Points are sampled uniformly over the annular sector's area; the radial component is sqrt-corrected, so an even spread is produced rather than a clustering near MinimumDistance. Must be >= 0.
	 */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	float MinimumDistance = 0.f;

	/** The maximum distance from Origin at which points may be generated. */
	UPROPERTY(Category = "Arc", BlueprintReadWrite)
	float MaximumDistance = 500.f;
};
