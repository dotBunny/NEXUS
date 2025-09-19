// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NFloatRange.generated.h"

/**
 * A float-based range structure.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNFloatRange
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Minimum = MIN_flt;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Maximum = MAX_flt;
	
	N_IMPLEMENT_RANGE(float)
};