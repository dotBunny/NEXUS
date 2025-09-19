// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NDoubleRange.generated.h"

/**
 * A double-based range structure.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNDoubleRange
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Minimum = MIN_dbl;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Maximum = MAX_dbl;

	N_IMPLEMENT_RANGE(double)
};