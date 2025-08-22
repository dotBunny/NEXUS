// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NDoubleRange.h"
#include "NDoubleRangeLibrary.generated.h"

/**
 * A Blueprint function library for working with <code>FNDoubleRange</code>.
 */
UCLASS()
class NEXUSCORE_API UNDoubleRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Double)", Category = "Nexus|Core|Range")
	static double NextValue(const FNDoubleRange& Range)
	{
		return Range.NextValue();
	}

	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Double)",  Category = "Nexus|Core|Range")
	static double NextValueInSubRange(const FNDoubleRange& Range, double MinimumValue, double MaximumValue)
	{
		return Range.NextValueInSubRange(MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Double)",  Category = "Nexus|Core|Range")
	static float PercentageValue(const FNDoubleRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Double)",  Category = "Nexus|Core|Range")
	static double RandomValueFromSeed(const FNDoubleRange& Range)
	{
		return Range.RandomValue();
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Double)",  Category = "Nexus|Core|Range")
	static double RandomValueInSubRange(const FNDoubleRange& Range, double MinimumValue, double MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value (Double)",  Category = "Nexus|Core|Range")
	static double RandomOneShotValue(const FNDoubleRange& Range, double Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Random One Shot Value In Sub-Range (Double)",  Category = "Nexus|Core|Range")
	static double RandomOneShotValueInSubRange(const FNDoubleRange& Range, int32 Seed, double MinimumValue, double MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Double)",  Category = "Nexus|Core|Range")
	static float ValuePercentage(const FNDoubleRange& Range, double Value)
	{
		return Range.ValuePercentage(Value);
	}
};