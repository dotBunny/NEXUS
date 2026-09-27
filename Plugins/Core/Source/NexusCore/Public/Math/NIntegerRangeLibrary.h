// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NIntegerRange.h"
#include "NMersenneTwisterObject.h"
#include "NIntegerRangeLibrary.generated.h"

/**
 * Blueprint-exposed wrappers around FNIntegerRange's sampling API.
 *
 * Thin passthroughs so that Blueprint authors can reach the same NextValue / RandomValue /
 * PercentageValue helpers that native code uses via N_RANGE_BASE.
 * @see <a href="https://nexus-framework.com/docs/core/types/math/integer-range-library/">UNIntegerRangeLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Integer Range Library")
class NEXUSCORE_API UNIntegerRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Deterministic sample from Range's full span.
	 * @param Range The range to sample.
	 * @param TwisterObject Twister supplying the sample; its stream advances by one draw.
	 * @return A value between Range's Minimum and Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Integer)", Category = "NEXUS|Core|Range")
	static int32 NextValue(const FNIntegerRange& Range, UNMersenneTwisterObject* TwisterObject)
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
	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Integer)",  Category = "NEXUS|Core|Range")
	static int32 NextValueInSubRange(const FNIntegerRange& Range, UNMersenneTwisterObject* TwisterObject, int32 MinimumValue, int32 MaximumValue)
	{
		return Range.NextValueInSubRange(TwisterObject->GetTwisterRef(), MinimumValue, MaximumValue);
	}

	/**
	 * Linearly interpolates between Range's Minimum and Maximum using Percentage (0..1).
	 * @param Range The range to interpolate across.
	 * @param Percentage 0 yields Minimum, 1 yields Maximum. Not clamped, so values outside 0..1 extrapolate.
	 * @return The interpolated value, truncated to a whole number by the range's integer arithmetic.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Integer)",  Category = "NEXUS|Core|Range")
	static float PercentageValue(const FNIntegerRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}

	/**
	 * Non-deterministic sample from Range's full span.
	 * @param Range The range to sample.
	 * @return A value between Range's Minimum and Maximum; differs from run to run.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomValue(const FNIntegerRange& Range)
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
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomValueInSubRange(const FNIntegerRange& Range, int32 MinimumValue, int32 MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	/**
	 * One-shot seeded sample from Range's full span; does not advance any persistent stream.
	 * @param Range The range to sample.
	 * @param Seed Seeds a fresh stream for this single draw, so the same Seed always yields the same value.
	 * @return A value between Range's Minimum and Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random One-Shot Value (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomOneShotValue(const FNIntegerRange& Range, const int32 Seed)
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
	UFUNCTION(BlueprintCallable, DisplayName="Random One-Shot Value In Sub-Range (Integer)",  Category = "NEXUS|Core|Range")
	static int32 RandomOneShotValueInSubRange(const FNIntegerRange& Range, int32 Seed, int32 MinimumValue, int32 MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	/**
	 * Returns Value's [0..1] position within Range (inverse of PercentageValue).
	 * @param Range The range to measure against.
	 * @param Value The value to locate.
	 * @return 0 at or below Minimum, 1 at or above Maximum, and 0 when Minimum equals Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Integer)",  Category = "NEXUS|Core|Range")
	static float ValuePercentage(const FNIntegerRange& Range, int32 Value)
	{
		return Range.ValuePercentage(Value);
	}
};