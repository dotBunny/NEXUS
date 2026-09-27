// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NFloatRange.h"
#include "NMersenneTwisterObject.h"
#include "NFloatRangeLibrary.generated.h"

/**
 * Blueprint-exposed wrappers around FNFloatRange's sampling API.
 *
 * Thin passthroughs so that Blueprint authors can reach the same NextValue / RandomValue /
 * PercentageValue helpers that native code uses via N_RANGE_BASE.
 * @see <a href="https://nexus-framework.com/docs/core/types/math/float-range-library/">UNFloatRangeLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Float Range Library")
class NEXUSCORE_API UNFloatRangeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Deterministic sample from Range's full span.
	 * @param Range The range to sample.
	 * @param TwisterObject Twister supplying the sample; its stream advances by one draw.
	 * @return A value between Range's Minimum and Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Next Value (Float)", Category = "NEXUS|Core|Range")
	static float NextValue(const FNFloatRange& Range, UNMersenneTwisterObject* TwisterObject)
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
	UFUNCTION(BlueprintCallable, DisplayName="Next Value In Sub-Range (Float)",  Category = "NEXUS|Core|Range")
	static float NextValueInSubRange(const FNFloatRange& Range, UNMersenneTwisterObject* TwisterObject, float MinimumValue, float MaximumValue)
	{
		return Range.NextValueInSubRange(TwisterObject->GetTwisterRef(), MinimumValue, MaximumValue);
	}

	/**
	 * Linearly interpolates between Range's Minimum and Maximum using Percentage (0..1).
	 * @param Range The range to interpolate across.
	 * @param Percentage 0 yields Minimum, 1 yields Maximum. Not clamped, so values outside 0..1 extrapolate.
	 * @return The interpolated value.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Percentage Value (Float)",  Category = "NEXUS|Core|Range")
	static float PercentageValue(const FNFloatRange& Range, float Percentage)
	{
		return Range.PercentageValue(Percentage);
	}

	/**
	 * Non-deterministic sample from Range's full span.
	 * @param Range The range to sample.
	 * @return A value between Range's Minimum and Maximum; differs from run to run.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random Value (Float)",  Category = "NEXUS|Core|Range")
	static float RandomValue(const FNFloatRange& Range)
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
	UFUNCTION(BlueprintCallable, DisplayName="Random Value In Sub-Range (Float)",  Category = "NEXUS|Core|Range")
	static float RandomValueInSubRange(const FNFloatRange& Range, float MinimumValue, float MaximumValue)
	{
		return Range.RandomValueInSubRange(MinimumValue, MaximumValue);
	}

	/**
	 * One-shot seeded sample from Range's full span; does not advance any persistent stream.
	 * @param Range The range to sample.
	 * @param Seed Seeds a fresh stream for this single draw, so the same Seed always yields the same value.
	 * @return A value between Range's Minimum and Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Random One-Shot Value (Float)",  Category = "NEXUS|Core|Range")
	static float RandomOneShotValue(const FNFloatRange& Range, const int32 Seed)
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
	UFUNCTION(BlueprintCallable, DisplayName="Random One-Shot Value In Sub-Range (Float)",  Category = "NEXUS|Core|Range")
	static float RandomOneShotValueInSubRange(const FNFloatRange& Range, int32 Seed, float MinimumValue, float MaximumValue)
	{
		return Range.RandomOneShotValueInSubRange(Seed, MinimumValue, MaximumValue);
	}

	/**
	 * Returns Value's [0..1] position within Range (inverse of PercentageValue).
	 * @param Range The range to measure against.
	 * @param Value The value to locate.
	 * @return 0 at or below Minimum, 1 at or above Maximum, and 0 when Minimum equals Maximum.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Value Percentage (Float)",  Category = "NEXUS|Core|Range")
	static float ValuePercentage(const FNFloatRange& Range, float Value)
	{
		return Range.ValuePercentage(Value);
	}
};