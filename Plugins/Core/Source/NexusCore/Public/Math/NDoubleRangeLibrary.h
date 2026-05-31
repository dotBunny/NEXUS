// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NDoubleRange.h"
#include "NDoubleRangeLibrary.generated.h"

/**
 * Blueprint-exposed wrappers around FNDoubleRange's sampling API.
 *
 * Thin passthroughs so that Blueprint authors can reach the same NextValue / RandomValue /
 * PercentageValue helpers that native code uses via N_IMPLEMENT_RANGE.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Double Range Library")
class NEXUSCORE_API UNDoubleRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** Deterministic sample from Range's full span. */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Double)", Category = "NEXUS|Core|Range")
	static double NextValue(const FNDoubleRange& Range)
	{
		return Range.NextValue();
	}

	/** Deterministic sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Double)",  Category = "NEXUS|Core|Range")
	static double NextValueInSubRange(const FNDoubleRange& Range, double MinimumValue, double MaximumValue)
	{
		return Range.NextValueInSubRange(MinimumValue, MaximumValue);
	}

	/** Linearly interpolates between Range's Minimum and Maximum using Percentage (0..1). */
	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Double)",  Category = "NEXUS|Core|Range")
	static float PercentageValue(const FNDoubleRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}

	/** Non-deterministic sample from Range's full span. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Double)",  Category = "NEXUS|Core|Range")
	static double RandomValueFromSeed(const FNDoubleRange& Range)
	{
		return Range.RandomValue();
	}

	/** Non-deterministic sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Double)",  Category = "NEXUS|Core|Range")
	static double RandomValueInSubRange(const FNDoubleRange& Range, double MinimumValue, double MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	/** One-shot seeded sample from Range's full span; does not advance any persistent stream. */
	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value (Double)",  Category = "NEXUS|Core|Range")
	static double RandomOneShotValue(const FNDoubleRange& Range, double Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}

	/** One-shot seeded sample clamped to [MinimumValue, MaximumValue] within Range. */
	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value In Sub-Range (Double)",  Category = "NEXUS|Core|Range")
	static double RandomOneShotValueInSubRange(const FNDoubleRange& Range, int32 Seed, double MinimumValue, double MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	/** Returns Value's [0..1] position within Range (inverse of PercentageValue). */
	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Double)",  Category = "NEXUS|Core|Range")
	static float ValuePercentage(const FNDoubleRange& Range, double Value)
	{
		return Range.ValuePercentage(Value);
	}
};