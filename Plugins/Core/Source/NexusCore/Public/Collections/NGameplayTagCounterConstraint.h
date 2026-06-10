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

	/**
	 * Tests whether a tag's current count in a counter satisfies this constraint.
	 * @param TagCounter The counter whose count for Tag is compared against Value via Comparison.
	 * @return true if the comparison holds; false otherwise, including when Comparison is unrecognized.
	 * @note A tag the counter does not track is compared as a count of zero.
	 */
	bool DoesPassComparison(const FNGameplayTagCounter& TagCounter) const
	{
		// TryGetValue leaves Current at zero for an untracked tag, so an absent tag compares as a count of zero
		// rather than asserting on a missing key the way GameplayTags[Tag] would.
		int32 Current = 0;
		TagCounter.TryGetValue(Tag, Current);

		switch (Comparison)
		{
		case ENComparisonResult::Equal:
			return Current == Value;

		case ENComparisonResult::GreaterThan:
			return Current > Value;

		case ENComparisonResult::GreaterThanOrEqual:
			return Current >= Value;

		case ENComparisonResult::LessThan:
			return Current < Value;

		case ENComparisonResult::LessThanOrEqual:
			return Current <= Value;

		case ENComparisonResult::NotEqual:
			return Current != Value;
		}
		
		return false;
	}
};
