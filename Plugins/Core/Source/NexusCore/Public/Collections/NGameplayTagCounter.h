// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "NGameplayTagCounter.generated.h"

/**
 * Tracks a running integer count per FGameplayTag.
 *
 * Used to accumulate how many times each tag has been seen, supporting increment, decrement, and merge operations.
 */
USTRUCT(BlueprintType)
struct FNGameplayTagCounter
{
	GENERATED_BODY()

	/** The per-tag counts; tags absent from the map are treated as a count of zero. */
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, int32> GameplayTags;

	/** Constructs an empty counter. */
	FNGameplayTagCounter()
	{
	}

	/**
	 * Constructs a counter seeded with an existing set of tag counts.
	 * @param ExistingCounters The tag/count pairs to copy in.
	 */
	explicit FNGameplayTagCounter(const TMap<FGameplayTag, int32>& ExistingCounters)
	{
		GameplayTags.Append(ExistingCounters);
	}

	/** Copy-constructs a counter from another, duplicating its counts. */
	FNGameplayTagCounter(const FNGameplayTagCounter& Other)
	{
		*this = Other;
	}

	/**
	 * Constructs a counter with a single tag already incremented to one.
	 * @param Tag The tag to seed with a count of one.
	 */
	explicit FNGameplayTagCounter(const FGameplayTag& Tag)
	{
		Increment(Tag);
	}

	/**
	 * Adds another counter's counts into this one, summing values for tags present in both.
	 * @param Other The counter whose counts should be merged in.
	 */
	void Combine(FNGameplayTagCounter& Other)
	{
		for (const TPair<FGameplayTag, int32>& Pair : Other.GameplayTags)
		{
			GameplayTags.FindOrAdd(Pair.Key) += Pair.Value;
		}
	}

	/**
	 * Increases the count for a tag by one, adding the tag if it is not already tracked.
	 * @param Tag The tag whose count should be incremented.
	 */
	void Increment(const FGameplayTag& Tag)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}	
		GameplayTags[Tag]++;
	}
	
	/**
	 * Decreases the count for a tag by one, adding the tag (then decrementing to -1) if it is not already tracked.
	 * @param Tag The tag whose count should be decremented.
	 */
	void Decrement(const FGameplayTag& Tag)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}
		GameplayTags[Tag]--;
	}

	/**
	 * Gets the current count for a tag.
	 * @param Tag The tag to query.
	 * @return The tracked count, or zero if the tag is not present.
	 */
	int32 GetCount(const FGameplayTag& Tag)
	{
		if (GameplayTags.Contains(Tag))
		{
			return GameplayTags[Tag];
		}
		return 0;
	}

	/**
	 * Does the collection contain an entry for a tag.
	 * @param Tag The tag to query for.
	 * @return true/false if the tag is present.
	 */
	bool Has(const FGameplayTag& Tag) const
	{
		return GameplayTags.Contains(Tag);
	}
};
