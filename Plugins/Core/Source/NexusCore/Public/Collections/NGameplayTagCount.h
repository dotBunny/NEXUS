// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NGameplayTagCount.generated.h"

/**
 * A single tag paired with its count.
 *
 * Flat, Blueprint-friendly representation of one entry in an FNGameplayTagCounter, used to serialize
 * or pass counts across boundaries where the underlying map is inconvenient.
 */
USTRUCT(BlueprintType)
struct FNGameplayTagCount
{
	GENERATED_BODY()

	/** The tag this count applies to. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTag Tag;

	/** The number of times the tag has been counted. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	int32 Count = 0;
};