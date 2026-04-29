// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NIntegerRange.h"
#include "NIntegerRangeLibrary.generated.h"

/**
 * Blueprint-exposed wrappers around FNIntegerRange's sampling API.
 *
 * Thin passthroughs so that Blueprint authors can reach the same NextValue / RandomValue /
 * PercentageValue helpers that native code uses via N_IMPLEMENT_RANGE.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Integer Range Library")
class NEXUSCORE_API UNIntegerRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** Deterministic sample from Range's full span. */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Integer)", Category = "NEXUS|Core|Range")
	static int32 NextValue(const FNIntegerRange& Range)
	{
		return Range.NextValue();
	}

	/** Deterministic sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Integer)",  Category = "NEXUS|Core|Range")
	static int32 NextValueInSubRange(const FNIntegerRange& Range, int32 MinimumValue, int32 MaximumValue)
	{
		return Range.NextValueInSubRange(MinimumValue, MaximumValue);
	}

	/** Linearly interpolates between Range's Minimum and Maximum using Percentage (0..1). */
	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Integer)",  Category = "NEXUS|Core|Range")
	static float PercentageValue(const FNIntegerRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}

	/** Non-deterministic sample from Range's full span. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomValueFromSeed(const FNIntegerRange& Range)
	{
		return Range.RandomValue();
	}

	/** Non-deterministic sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomValueInSubRangeFromSeed(const FNIntegerRange& Range, int32 MinimumValue, int32 MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	/** One-shot seeded sample from Range's full span; does not advance any persistent stream. */
	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value(Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomOneShotValue(const FNIntegerRange& Range, const int32 Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}

	/** One-shot seeded sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value In Sub-Range (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomOneShotValueInSubRange(const FNIntegerRange& Range, int32 Seed, int32 MinimumValue, int32 MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	/** Returns Value's [0..1] position within Range (inverse of PercentageValue). */
	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Integer)",  Category = "NEXUS|Core|Range")
	static float ValuePercentage(const FNIntegerRange& Range, int32 Value)
	{
		return Range.ValuePercentage(Value);
	}
};