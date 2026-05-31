// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NDoubleRange.generated.h"

/**
 * A double-precision inclusive range [Minimum, Maximum].
 *
 * Defaults span the entire representable range of double; override either bound at author time.
 * The member API (NextValue, RandomValue, PercentageValue, etc.) is supplied by N_IMPLEMENT_RANGE.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNDoubleRange
{
	GENERATED_BODY()

	/** Lower bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Minimum = MIN_dbl;

	/** Upper bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Maximum = MAX_dbl;

	N_IMPLEMENT_RANGE(double)
};