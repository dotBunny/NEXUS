// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NIntegerRange.generated.h"

/**
 * A 32-bit integer inclusive range [Minimum, Maximum].
 *
 * Defaults span the entire representable range of int32; override either bound at author time.
 * The member API (NextValue, RandomValue, PercentageValue, etc.) is supplied by N_IMPLEMENT_RANGE.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNIntegerRange
{
	GENERATED_BODY()

	/** Lower bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Minimum = MIN_int32;

	/** Upper bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Maximum = MAX_int32;

	N_IMPLEMENT_RANGE(int32)
};