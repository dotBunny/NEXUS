// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NMersenneTwister.h"
#include "NDoubleRange.generated.h"

/**
 * A double-precision inclusive range [Minimum, Maximum].
 *
 * Defaults to [-MIN_dbl, MAX_dbl]; override both bounds at author time.
 * The member API (NextValue, RandomValue, PercentageValue, etc.) is supplied by N_RANGE_BASE.
 * @note MIN_dbl is the smallest positive normalized double rather than the most negative one, so the default
 *       lower bound is effectively zero. Widening it to -MAX_dbl is deliberately avoided: sampling evaluates
 *       (Maximum - Minimum), which overflows to infinity across the full representable span.
 * @note The Random* methods are half-open [Minimum, Maximum); the range itself is inclusive for the
 *       clamping and percentage methods.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/math/double-range/">FNDoubleRange</a>
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNDoubleRange
{
	GENERATED_BODY()

	/** Lower bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Minimum = -MIN_dbl;

	/** Upper bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Maximum = MAX_dbl;

	N_RANGE_BASE(double)
};