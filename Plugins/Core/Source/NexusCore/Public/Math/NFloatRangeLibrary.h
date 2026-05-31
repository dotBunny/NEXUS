// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NFloatRange.h"
#include "NFloatRangeLibrary.generated.h"

/**
 * Blueprint-exposed wrappers around FNFloatRange's sampling API.
 *
 * Thin passthroughs so that Blueprint authors can reach the same NextValue / RandomValue /
 * PercentageValue helpers that native code uses via N_IMPLEMENT_RANGE.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Float Range Library")
class NEXUSCORE_API UNFloatRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** Deterministic sample from Range's full span. */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Float)", Category = "NEXUS|Core|Range")
	static float NextValue(const FNFloatRange& Range)
	{
		return Range.NextValue();
	}

	/** Deterministic sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Float)",  Category = "NEXUS|Core|Range")
	static float NextValueInSubRange(const FNFloatRange& Range, float MinimumValue, float MaximumValue)
	{
		return Range.NextValueInSubRange(MinimumValue, MaximumValue);
	}

	/** Linearly interpolates between Range's Minimum and Maximum using Percentage (0..1). */
	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Float)",  Category = "NEXUS|Core|Range")
	static float PercentageValue(const FNFloatRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}

	/** Non-deterministic sample from Range's full span. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Float)",  Category = "NEXUS|Core|Range")
	static float RandomValue(const FNFloatRange& Range)
	{
		return Range.RandomValue();
	}

	/** Non-deterministic sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Float)",  Category = "NEXUS|Core|Range")
	static float RandomValueInSubRange(const FNFloatRange& Range, float MinimumValue, float MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	/** One-shot seeded sample from Range's full span; does not advance any persistent stream. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value One Shot (Float)",  Category = "NEXUS|Core|Range")
	static float RandomOneShotValue(const FNFloatRange& Range, float Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}

	/** One-shot seeded sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value In Sub-Range (Float)",  Category = "NEXUS|Core|Range")
	static float RandomOneShotValueInSubRange(const FNFloatRange& Range, int32 Seed, float MinimumValue, float MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	/** Returns Value's [0..1] position within Range (inverse of PercentageValue). */
	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Float)",  Category = "NEXUS|Core|Range")
	static float ValuePercentage(const FNFloatRange& Range, float Value)
	{
		return Range.ValuePercentage(Value);
	}
};