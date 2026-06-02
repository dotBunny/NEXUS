// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NGameplayTagCounter.h"
#include "Types/NComparisonResult.h"
#include "NGameplayTagCounterConstraint.generated.h"

/**
 * A declarative predicate tested against a tag's count within an FNGameplayTagCounter.
 *
 * Authored in the editor to gate behavior on a tag's running total (e.g. "count >= 1") when the
 * constraint is evaluated.
 */
USTRUCT(BlueprintType)
struct FNGameplayTagCounterConstraint
{
	GENERATED_BODY()

	/** The tag whose count this constraint tests. */
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	/** How the tag's current count is compared against Value. */
	UPROPERTY(EditAnywhere)
	ENComparisonResult Comparison = ENComparisonResult::GreaterThanOrEqual;

	/** The value the tag's current count is compared against via Comparison. */
	UPROPERTY(EditAnywhere)
	int32 Value = 1;
	
	bool Evaluate(const FNGameplayTagCounter& TagCounter)
	{
		
		return false;
	}
};
