// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"
#include "NRangeMacros.h"
#include "NIntegerRange.generated.h"

/**
 * An <code>int32</code> based range structure.
 */
USTRUCT(BlueprintType)
struct NEXUSCORE_API FNIntegerRange
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Minimum = MIN_int32;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Maximum = MAX_int32;
	
	N_IMPLEMENT_RANGE(int)
};