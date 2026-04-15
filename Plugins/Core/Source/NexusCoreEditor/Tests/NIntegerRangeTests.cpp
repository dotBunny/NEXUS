// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NIntegerRange.h"
#include "Math/NFloatRange.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNIntegerRangeTests_PercentageValue_Extremes, "NEXUS::UnitTests::NCore::FNIntegerRange::PercentageValue_Extremes", N_TEST_CONTEXT_ANYWHERE)
{
	FNIntegerRange Range;
	Range.Minimum = 0;
	Range.Maximum = 100;
	CHECK_EQUALS("0% should return Minimum", Range.PercentageValue(0.f), 0);
	CHECK_EQUALS("100% should return Maximum", Range.PercentageValue(1.f), 100);
}

N_TEST_HIGH(FNIntegerRangeTests_PercentageValue_Midpoint, "NEXUS::UnitTests::NCore::FNIntegerRange::PercentageValue_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNIntegerRange Range;
	Range.Minimum = 0;
	Range.Maximum = 200;
	CHECK_EQUALS("50% of 0..200 should be 100", Range.PercentageValue(0.5f), 100);
}

N_TEST_HIGH(FNIntegerRangeTests_ValuePercentage_Extremes, "NEXUS::UnitTests::NCore::FNIntegerRange::ValuePercentage_Extremes", N_TEST_CONTEXT_ANYWHERE)
{
	FNIntegerRange Range;
	Range.Minimum = 0;
	Range.Maximum = 100;
	CHECK_MESSAGE(TEXT("Minimum should map to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(0), 0.f, 0.001f));
	CHECK_MESSAGE(TEXT("Maximum should map to 100%"), FMath::IsNearlyEqual(Range.ValuePercentage(100), 1.f, 0.001f));
}

N_TEST_HIGH(FNIntegerRangeTests_ValuePercentage_BelowMinimum, "NEXUS::UnitTests::NCore::FNIntegerRange::ValuePercentage_BelowMinimum", N_TEST_CONTEXT_ANYWHERE)
{
	FNIntegerRange Range;
	Range.Minimum = 10;
	Range.Maximum = 20;
	CHECK_MESSAGE(TEXT("Value below minimum should clamp to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(5), 0.f, 0.001f));
}

N_TEST_HIGH(FNIntegerRangeTests_ValuePercentage_AboveMaximum, "NEXUS::UnitTests::NCore::FNIntegerRange::ValuePercentage_AboveMaximum", N_TEST_CONTEXT_ANYWHERE)
{
	FNIntegerRange Range;
	Range.Minimum = 10;
	Range.Maximum = 20;
	CHECK_MESSAGE(TEXT("Value above maximum should clamp to 100%"), FMath::IsNearlyEqual(Range.ValuePercentage(30), 1.f, 0.001f));
}

N_TEST_HIGH(FNIntegerRangeTests_NextValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNIntegerRange::NextValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNIntegerRange Range;
	Range.Minimum = 10;
	Range.Maximum = 50;
	for (int32 i = 0; i < 100; ++i)
	{
		const int32 Value = Range.NextValueInSubRange(0, 100);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 10);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 50);
	}
}

N_TEST_HIGH(FNFloatRangeTests_PercentageValue_Midpoint, "NEXUS::UnitTests::NCore::FNFloatRange::PercentageValue_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 10.f;
	CHECK_MESSAGE(TEXT("50% of 0..10 should be 5"), FMath::IsNearlyEqual(Range.PercentageValue(0.5f), 5.f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_ValuePercentage_Midpoint, "NEXUS::UnitTests::NCore::FNFloatRange::ValuePercentage_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 10.f;
	CHECK_MESSAGE(TEXT("5 of 0..10 should be 50%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.f), 0.5f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_RandomValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNFloatRange::RandomValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 5.f;
	Range.Maximum = 15.f;
	for (int32 i = 0; i < 100; ++i)
	{
		const float Value = Range.RandomValueInSubRange(0.f, 100.f);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 5.f - 0.001f);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 15.f + 0.001f);
	}
}

N_TEST_MEDIUM(FNFloatRangeTests_RandomOneShotValue_Determinism, "NEXUS::UnitTests::NCore::FNFloatRange::RandomOneShotValue_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 100.f;
	const float A = Range.RandomOneShotValue(12345);
	const float B = Range.RandomOneShotValue(12345);
	CHECK_MESSAGE(TEXT("Same seed should produce same OneShot value"), FMath::IsNearlyEqual(A, B));
}

#endif //WITH_TESTS
