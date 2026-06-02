// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "NGameplayTagCounterOperationType.h"
#include "NGameplayTagCounterOperation.generated.h"

/**
 * A declarative mutation to apply to a tag's count within an FNGameplayTagCounter.
 *
 * Authored in the editor to describe how a tag's running total should change (e.g. add, subtract)
 * when the operation is evaluated.
 */
USTRUCT(BlueprintType)
struct FNGameplayTagCounterOperation
{
	GENERATED_BODY()

	/** The tag whose count this operation mutates. */
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	/** The arithmetic operation applied to the tag's current count using Value. */
	UPROPERTY(EditAnywhere)
	ENGameplayTagCounterOperationType Operation = ENGameplayTagCounterOperationType::Add;

	/** The right-hand operand combined with the tag's current count via Operation. */
	UPROPERTY(EditAnywhere)
	int32 Value = 0;
};