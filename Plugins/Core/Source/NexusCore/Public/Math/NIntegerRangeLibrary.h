// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NIntegerRange.h"
#include "NIntegerRangeLibrary.generated.h"

/**
 * A Blueprint function library for working with FNIntegerRange.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Integer Range Library")
class NEXUSCORE_API UNIntegerRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Integer)", Category = "Nexus|Core|Range")
	static int NextValue(const FNIntegerRange& Range)
	{
		return Range.NextValue();
	}

	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Integer)",  Category = "Nexus|Core|Range")
	static int NextValueInSubRange(const FNIntegerRange& Range, int MinimumValue, int MaximumValue)
	{
		return Range.NextValueInSubRange(MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Integer)",  Category = "Nexus|Core|Range")
	static float PercentageValue(const FNIntegerRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Integer)",  Category = "Nexus|Core|Range")
	static int RandomValueFromSeed(const FNIntegerRange& Range)
	{
		return Range.RandomValue();
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Integer)",  Category = "Nexus|Core|Range")
	static int RandomValueInSubRangeFromSeed(const FNIntegerRange& Range, int MinimumValue, int MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value(Integer)",  Category = "Nexus|Core|Range")
	static int RandomOneShotValue(const FNIntegerRange& Range, const int Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value In Sub-Range (Integer)",  Category = "Nexus|Core|Range")
	static int RandomOneShotValueInSubRange(const FNIntegerRange& Range, int Seed, int MinimumValue, int MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Integer)",  Category = "Nexus|Core|Range")
	static float ValuePercentage(const FNIntegerRange& Range, int Value)
	{
		return Range.ValuePercentage(Value);
	}
};