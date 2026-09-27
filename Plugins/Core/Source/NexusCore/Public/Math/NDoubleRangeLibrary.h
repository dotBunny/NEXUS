// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NDoubleRange.h"
#include "NMersenneTwisterObject.h"
#include "NDoubleRangeLibrary.generated.h"

/**
 * Blueprint-exposed wrappers around FNDoubleRange's sampling API.
 *
 * Thin passthroughs so that Blueprint authors can reach the same NextValue / RandomValue /
 * PercentageValue helpers that native code uses via N_RANGE_BASE.
 * @see <a href="https://nexus-framework.com/docs/core/types/math/double-range-library/">UNDoubleRangeLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Double Range Library")
class NEXUSCORE_API UNDoubleRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Deterministic sample from Range's full span.
	 * @param Range The range to sample.
	 * @param TwisterObject Twister supplying the sample; its stream advances by one draw.
	 * @return A value between Range's Minimum and Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Double)", Category = "NEXUS|Core|Range")
	static double NextValue(const FNDoubleRange& Range, UNMersenneTwisterObject* TwisterObject)
	{
		return Range.NextValue(TwisterObject->GetTwisterRef());
	}

	/**
	 * Deterministic sample clamped to [MinimumValue, MaximumValue] within Range.
	 * @param Range The range to sample.
	 * @param TwisterObject Twister supplying the sample; its stream advances by one draw.
	 * @param MinimumValue Lower bound of the sub-range; raised to Range's Minimum when below it.
	 * @param MaximumValue Upper bound of the sub-range; lowered to Range's Maximum when above it.
	 * @return A value within the clamped sub-range.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Double)",  Category = "NEXUS|Core|Range")
	static double NextValueInSubRange(const FNDoubleRange& Range, UNMersenneTwisterObject* TwisterObject, double MinimumValue, double MaximumValue)
	{
		return Range.NextValueInSubRange(TwisterObject->GetTwisterRef(), MinimumValue, MaximumValue);
	}

	/**
	 * Linearly interpolates between Range's Minimum and Maximum using Percentage (0..1).
	 * @param Range The range to interpolate across.
	 * @param Percentage 0 yields Minimum, 1 yields Maximum. Not clamped, so values outside 0..1 extrapolate.
	 * @return The interpolated value as a float, so a double range loses precision here.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Double)",  Category = "NEXUS|Core|Range")
	static float PercentageValue(const FNDoubleRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}

	/**
	 * Non-deterministic sample from Range's full span.
	 * @param Range The range to sample.
	 * @return A value between Range's Minimum and Maximum; differs from run to run.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Double)",  Category = "NEXUS|Core|Range")
	static double RandomValue(const FNDoubleRange& Range)
	{
		return Range.RandomValue();
	}

	/**
	 * Non-deterministic sample clamped to [MinimumValue, MaximumValue] within Range.
	 * @param Range The range to sample.
	 * @param MinimumValue Lower bound of the sub-range; raised to Range's Minimum when below it.
	 * @param MaximumValue Upper bound of the sub-range; lowered to Range's Maximum when above it.
	 * @return A value within the clamped sub-range; differs from run to run.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Double)",  Category = "NEXUS|Core|Range")
	static double RandomValueInSubRange(const FNDoubleRange& Range, double MinimumValue, double MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	/**
	 * One-shot seeded sample from Range's full span; does not advance any persistent stream.
	 * @param Range The range to sample.
	 * @param Seed Seeds a fresh stream for this single draw, so the same Seed always yields the same value.
	 * @return A value between Range's Minimum and Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random One-Shot Value (Double)",  Category = "NEXUS|Core|Range")
	static double RandomOneShotValue(const FNDoubleRange& Range, const int32 Seed)
	{
		return Range.RandomOneShotValue(Seed);
	}

	/**
	 * One-shot seeded sample clamped to [MinimumValue, MaximumValue] within Range.
	 * @param Range The range to sample.
	 * @param Seed Seeds a fresh stream for this single draw, so the same Seed always yields the same value.
	 * @param MinimumValue Lower bound of the sub-range; raised to Range's Minimum when below it.
	 * @param MaximumValue Upper bound of the sub-range; lowered to Range's Maximum when above it.
	 * @return A value within the clamped sub-range.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random One-Shot Value In Sub-Range (Double)",  Category = "NEXUS|Core|Range")
	static double RandomOneShotValueInSubRange(const FNDoubleRange& Range, int32 Seed, double MinimumValue, double MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	/**
	 * Returns Value's [0..1] position within Range (inverse of PercentageValue).
	 * @param Range The range to measure against.
	 * @param Value The value to locate.
	 * @return 0 at or below Minimum, 1 at or above Maximum, and 0 when Minimum equals Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Double)",  Category = "NEXUS|Core|Range")
	static float ValuePercentage(const FNDoubleRange& Range, double Value)
	{
		return Range.ValuePercentage(Value);
	}
};