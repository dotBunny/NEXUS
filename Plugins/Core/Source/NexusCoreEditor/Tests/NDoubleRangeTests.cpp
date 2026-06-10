// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NDoubleRange.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNDoubleRangeTests_PercentageValue_Extremes, "NEXUS::UnitTests::NCore::FNDoubleRange::PercentageValue_Extremes", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = -25.0;
	Range.Maximum = 75.0;
	CHECK_MESSAGE(TEXT("0% should return Minimum"), FMath::IsNearlyEqual(Range.PercentageValue(0.f), -25.0, 0.001));
	CHECK_MESSAGE(TEXT("100% should return Maximum"), FMath::IsNearlyEqual(Range.PercentageValue(1.f), 75.0, 0.001));
}

N_TEST_HIGH(FNDoubleRangeTests_PercentageValue_Midpoint, "NEXUS::UnitTests::NCore::FNDoubleRange::PercentageValue_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 0.0;
	Range.Maximum = 10.0;
	CHECK_MESSAGE(TEXT("50% of 0..10 should be 5"), FMath::IsNearlyEqual(Range.PercentageValue(0.5f), 5.0, 0.001));
}

N_TEST_HIGH(FNDoubleRangeTests_ValuePercentage_Extremes, "NEXUS::UnitTests::NCore::FNDoubleRange::ValuePercentage_Extremes", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 0.0;
	Range.Maximum = 100.0;
	CHECK_MESSAGE(TEXT("Minimum should map to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(0.0), 0.f, 0.001f));
	CHECK_MESSAGE(TEXT("Maximum should map to 100%"), FMath::IsNearlyEqual(Range.ValuePercentage(100.0), 1.f, 0.001f));
}

N_TEST_HIGH(FNDoubleRangeTests_ValuePercentage_Midpoint, "NEXUS::UnitTests::NCore::FNDoubleRange::ValuePercentage_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 0.0;
	Range.Maximum = 10.0;
	CHECK_MESSAGE(TEXT("5 of 0..10 should be 50%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.0), 0.5f, 0.001f));

	Range.Minimum = -50.0;
	Range.Maximum = 50.0;
	CHECK_MESSAGE(TEXT("Midpoint of -50..50 should map to 50%"), FMath::IsNearlyEqual(Range.ValuePercentage(0.0), 0.5f, 0.001f));
	CHECK_MESSAGE(TEXT("Quartile of -50..50 should map to 25%"), FMath::IsNearlyEqual(Range.ValuePercentage(-25.0), 0.25f, 0.001f));
}

N_TEST_HIGH(FNDoubleRangeTests_ValuePercentage_BelowMinimum, "NEXUS::UnitTests::NCore::FNDoubleRange::ValuePercentage_BelowMinimum", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 10.0;
	Range.Maximum = 20.0;
	CHECK_MESSAGE(TEXT("Value below minimum should clamp to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.0), 0.f, 0.001f));
}

N_TEST_HIGH(FNDoubleRangeTests_ValuePercentage_AboveMaximum, "NEXUS::UnitTests::NCore::FNDoubleRange::ValuePercentage_AboveMaximum", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 10.0;
	Range.Maximum = 20.0;
	CHECK_MESSAGE(TEXT("Value above maximum should clamp to 100%"), FMath::IsNearlyEqual(Range.ValuePercentage(30.0), 1.f, 0.001f));
}

N_TEST_MEDIUM(FNDoubleRangeTests_ValuePercentage_DegenerateRange, "NEXUS::UnitTests::NCore::FNDoubleRange::ValuePercentage_DegenerateRange", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that a zero-width range returns 0% rather than dividing by zero.
	FNDoubleRange Range;
	Range.Minimum = 5.0;
	Range.Maximum = 5.0;
	CHECK_MESSAGE(TEXT("A zero-width range should map its value to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.0), 0.f, 0.001f));
}

N_TEST_HIGH(FNDoubleRangeTests_NextValue_WithinBounds, "NEXUS::UnitTests::NCore::FNDoubleRange::NextValue_WithinBounds", N_TEST_CONTEXT_ANYWHERE)
{
	// Fractional bounds are intentional: NextValue samples through FNMersenneTwister::RandRange,
	// which has a true double overload (unlike the FRandomStream-backed Random* methods).
	FNDoubleRange Range;
	Range.Minimum = 0.25;
	Range.Maximum = 0.75;
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.NextValue();
		CHECK_MESSAGE(FString::Printf(TEXT("NextValue[%d] should be >= Minimum"), i), Value >= 0.25 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("NextValue[%d] should be <= Maximum"), i), Value <= 0.75 + 0.001);
	}
}

N_TEST_HIGH(FNDoubleRangeTests_NextValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNDoubleRange::NextValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 5.5;
	Range.Maximum = 15.5;
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.NextValueInSubRange(0.0, 100.0);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 5.5 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 15.5 + 0.001);
	}
}

N_TEST_HIGH(FNDoubleRangeTests_RandomValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNDoubleRange::RandomValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	// Bounds are whole numbers: the Random* methods sample through FRandomStream::RandRange,
	// which only has an int32 overload, so fractional bounds would truncate.
	FNDoubleRange Range;
	Range.Minimum = 5.0;
	Range.Maximum = 15.0;
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.RandomValueInSubRange(0.0, 100.0);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 5.0 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 15.0 + 0.001);
	}
}

N_TEST_MEDIUM(FNDoubleRangeTests_RandomOneShotValue_Determinism, "NEXUS::UnitTests::NCore::FNDoubleRange::RandomOneShotValue_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 0.0;
	Range.Maximum = 100.0;
	const double A = Range.RandomOneShotValue(12345);
	const double B = Range.RandomOneShotValue(12345);
	CHECK_MESSAGE(TEXT("Same seed should produce same OneShot value"), FMath::IsNearlyEqual(A, B));
}

N_TEST_MEDIUM(FNDoubleRangeTests_RandomOneShotValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNDoubleRange::RandomOneShotValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 10.0;
	Range.Maximum = 20.0;
	const double A = Range.RandomOneShotValueInSubRange(4321, 0.0, 100.0);
	const double B = Range.RandomOneShotValueInSubRange(4321, 0.0, 100.0);
	CHECK_MESSAGE(TEXT("Same seed should produce same sub-range OneShot value"), FMath::IsNearlyEqual(A, B));
	CHECK_MESSAGE(TEXT("Sub-range OneShot value should be >= Range.Minimum"), A >= 10.0 - 0.001);
	CHECK_MESSAGE(TEXT("Sub-range OneShot value should be <= Range.Maximum"), A <= 20.0 + 0.001);
}

N_TEST_MEDIUM(FNDoubleRangeTests_RandomTrackedValue_Determinism, "NEXUS::UnitTests::NCore::FNDoubleRange::RandomTrackedValue_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 0.0;
	Range.Maximum = 100.0;
	int32 SeedA = 777;
	int32 SeedB = 777;
	const double A = Range.RandomTrackedValue(SeedA);
	const double B = Range.RandomTrackedValue(SeedB);
	CHECK_MESSAGE(TEXT("Same starting seed should produce same tracked value"), FMath::IsNearlyEqual(A, B));
	CHECK_MESSAGE(TEXT("Identical starting seeds should advance to identical values"), SeedA == SeedB);
	CHECK_MESSAGE(TEXT("Tracked sampling should advance the seed"), SeedA != 777);
}

N_TEST_MEDIUM(FNDoubleRangeTests_RandomTrackedValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNDoubleRange::RandomTrackedValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 10.0;
	Range.Maximum = 20.0;
	int32 Seed = 246;
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.RandomTrackedValueInSubRange(Seed, 0.0, 15.0);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 10.0 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= the sub-range maximum"), i), Value <= 15.0 + 0.001);
	}
}

#endif //WITH_TESTS
