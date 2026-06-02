// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "Types/NComparisonResult.h"
#include "NGameplayTagCounter.generated.h"


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


UENUM(BlueprintType)
enum class ENGameplayTagCounterOperationType : uint8
{
	Add = 0,
	Subtract = 1,
};

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
};


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
	
	/**
	 * Constructs a counter seeded from a flat array of tag/count pairs.
	 * @param Counts The tag/count entries to copy in; duplicate tags are summed.
	 */
	explicit FNGameplayTagCounter(const TArray<FNGameplayTagCount>& Counts)
	{
		GameplayTags.Reserve(Counts.Num());
		for (const FNGameplayTagCount& Entry : Counts)
		{
			GameplayTags.FindOrAdd(Entry.Tag) += Entry.Count;
		}
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
	void Combine(const FNGameplayTagCounter& Other)
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
	 * Adds a value to the count for a tag, adding the tag (starting from zero) if it is not already tracked.
	 * @param Tag The tag whose count should be increased.
	 * @param Value The amount to add to the tag's current count.
	 */
	void Add(const FGameplayTag& Tag, const int32 Value)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}
		GameplayTags[Tag] += Value;
	}

	/**
	 * Subtracts a value from the count for a tag, adding the tag (starting from zero) if it is not already tracked.
	 * @param Tag The tag whose count should be decreased.
	 * @param Value The amount to subtract from the tag's current count.
	 */
	void Subtract(const FGameplayTag& Tag, const int32 Value)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}
		GameplayTags[Tag] -= Value;
	}

	/**
	 * Multiplies the count for a tag by a value, adding the tag (starting from zero) if it is not already tracked.
	 * @param Tag The tag whose count should be multiplied.
	 * @param Value The factor to multiply the tag's current count by.
	 * @note Multiplying a freshly-added tag yields zero, since absent tags start at a count of zero.
	 */
	void Multiply(const FGameplayTag& Tag, const int32 Value)
	{
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}
		GameplayTags[Tag] *= Value;
	}

	/**
	 * Divides the count for a tag by a value, adding the tag (starting from zero) if it is not already tracked.
	 * @param Tag The tag whose count should be divided.
	 * @param Value The divisor to divide the tag's current count by.
	 */
	void Divide(const FGameplayTag& Tag, const int32 Value)
	{
		// Protect against divide by zero bs
		if (Value == 0) return;
		
		if (!GameplayTags.Contains(Tag))
		{
			GameplayTags.Add(Tag);
		}
		GameplayTags[Tag] /= Value;
	}
	
	void ApplyOperation(const FNGameplayTagCounterOperation& Operation)
	{
		switch (Operation.Operation)
		{
		case ENGameplayTagCounterOperationType::Add:
			Add(Operation.Tag, Operation.Value);
			break;
		case ENGameplayTagCounterOperationType::Subtract:
			Subtract(Operation.Tag, Operation.Value);
			break;
		}
	}
	
	void ReverseOperation(const FNGameplayTagCounterOperation& Operation)
	{
		switch (Operation.Operation)
		{
		case ENGameplayTagCounterOperationType::Add:
			Subtract(Operation.Tag, Operation.Value);
			break;
		case ENGameplayTagCounterOperationType::Subtract:
			Add(Operation.Tag, Operation.Value);
			break;
		}
	}

	/**
	 * Gets the current value for a tag.
	 * @param Tag The tag to query.
	 * @return The tracked count, or zero if the tag is not present.
	 */
	int32 GetValue(const FGameplayTag& Tag)
	{
		if (GameplayTags.Contains(Tag))
		{
			return GameplayTags[Tag];
		}
		return 0;
	}
	
	/**
	 * Attempts to read the current value for a tag without treating an absent tag as zero.
	 * @param Tag The tag to query.
	 * @param OutValue Receives the tracked count when the tag is present; left unchanged otherwise.
	 * @return true if the tag is tracked and OutValue was written; false if the tag is absent.
	 */
	bool TryGetValue(const FGameplayTag& Tag, int32& OutValue)
	{
		if (!GameplayTags.Contains(Tag))
		{
			return false;
		}
		OutValue = GameplayTags[Tag];
		return true;
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
	
	/**
	 * Builds a flat array of tag/count pairs from the tracked counts.
	 * @return An array containing one entry per tracked tag, in unspecified order.
	 */
	TArray<FNGameplayTagCount> ToTagCount() const
	{
		TArray<FNGameplayTagCount> ReturnArray;
		ReturnArray.Reserve(GameplayTags.Num());
		for (const TPair<FGameplayTag, int32>& Pair : GameplayTags)
		{
			FNGameplayTagCount& Entry = ReturnArray.AddDefaulted_GetRef();
			Entry.Tag = Pair.Key;
			Entry.Count = Pair.Value;
		}
		return ReturnArray;
	}
	
	/**
	 * Builds a counter holding the signed difference of this counter minus another.
	 *
	 * Every tag's result is this counter's count minus Other's, treating an absent tag as zero on either
	 * side. The result is antisymmetric: A.GetDifference(B) is the negation of B.GetDifference(A).
	 * @param Other The counter to subtract from this one.
	 * @return A new counter where each tag holds this count minus Other's count.
	 */
	FNGameplayTagCounter GetDifference(const FNGameplayTagCounter& Other) const
	{
		// Seed with our own counts; any tag we hold that Other lacks then passes through unchanged.
		FNGameplayTagCounter ReturnValue;
		ReturnValue.GameplayTags = GameplayTags;

		for (const TPair<FGameplayTag, int32>& Pair : Other.GameplayTags)
		{
			// Present in both: subtract Other's count. Present only in Other: this side is zero, so the
			// result is -Other (FindOrAdd seeds the entry at zero before subtracting).
			ReturnValue.GameplayTags.FindOrAdd(Pair.Key) -= Pair.Value;
		}

		return ReturnValue;
	}
};
