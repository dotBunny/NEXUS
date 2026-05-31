// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NFloatRange.generated.h"

/**
 * A single-precision inclusive range [Minimum, Maximum].
 *
 * Defaults span the entire representable range of float; override either bound at author time.
 * The member API (NextValue, RandomValue, PercentageValue, etc.) is supplied by N_IMPLEMENT_RANGE.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNFloatRange
{
	GENERATED_BODY()

	/** Lower bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Minimum = MIN_flt;

	/** Upper bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Maximum = MAX_flt;

	N_IMPLEMENT_RANGE(float)
};