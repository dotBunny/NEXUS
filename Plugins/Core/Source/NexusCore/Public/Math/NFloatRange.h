// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NMersenneTwister.h"
#include "NFloatRange.generated.h"

/**
 * A single-precision inclusive range [Minimum, Maximum].
 *
 * Defaults to [-MIN_flt, MAX_flt]; override both bounds at author time.
 * The member API (NextValue, RandomValue, PercentageValue, etc.) is supplied by N_RANGE_BASE.
 * @note MIN_flt is the smallest positive normalized float rather than the most negative one, so the default
 *       lower bound is effectively zero. Widening it to -MAX_flt is deliberately avoided: sampling evaluates
 *       (Maximum - Minimum), which overflows across the full representable span.
 * @note The Random* methods are half-open [Minimum, Maximum); the range itself is inclusive for the
 *       clamping and percentage methods.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/math/float-range/">FNFloatRange</a>
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNFloatRange
{
	GENERATED_BODY()

	/** Lower bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Minimum = -MIN_flt;

	/** Upper bound of the range (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Maximum = MAX_flt;

	N_RANGE_BASE(float)
};