// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NFloatRange.h"
#include "NFloatRangeLibrary.generated.h"

/**
 * A Blueprint function library for working with FNFloatRange.
 */
UCLASS(DisplayName = "NEXUS: Float Range Library")
class NEXUSCORE_API UNFloatRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Float)", Category = "Nexus|Core|Range")
	static float NextValue(const FNFloatRange& Range)
	{
		return Range.NextValue();
	}

	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Float)",  Category = "Nexus|Core|Range")
	static float NextValueInSubRange(const FNFloatRange& Range, float MinimumValue, float MaximumValue)
	{
		return Range.NextValueInSubRange(MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Float)",  Category = "Nexus|Core|Range")
	static float PercentageValue(const FNFloatRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Float)",  Category = "Nexus|Core|Range")
	static float RandomValue(const FNFloatRange& Range)
	{
		return Range.RandomValue();
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Float)",  Category = "Nexus|Core|Range")
	static float RandomValueInSubRange(const FNFloatRange& Range, float MinimumValue, float MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random Value One Shot (Float)",  Category = "Nexus|Core|Range")
	static float RandomOneShotValue(const FNFloatRange& Range, float Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value In Sub-Range (Float)",  Category = "Nexus|Core|Range")
	static float RandomOneShotValueInSubRange(const FNFloatRange& Range, int32 Seed, float MinimumValue, float MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Float)",  Category = "Nexus|Core|Range")
	static float ValuePercentage(const FNFloatRange& Range, float Value)
	{
		return Range.ValuePercentage(Value);
	}
};