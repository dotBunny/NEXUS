// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NFloatRange.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNFloatRangeTests_PercentageValue_Extremes, "NEXUS::UnitTests::NCore::FNFloatRange::PercentageValue_Extremes", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = -25.f;
	Range.Maximum = 75.f;
	CHECK_MESSAGE(TEXT("0% should return Minimum"), FMath::IsNearlyEqual(Range.PercentageValue(0.f), -25.f, 0.001f));
	CHECK_MESSAGE(TEXT("100% should return Maximum"), FMath::IsNearlyEqual(Range.PercentageValue(1.f), 75.f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_PercentageValue_Midpoint, "NEXUS::UnitTests::NCore::FNFloatRange::PercentageValue_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 10.f;
	CHECK_MESSAGE(TEXT("50% of 0..10 should be 5"), FMath::IsNearlyEqual(Range.PercentageValue(0.5f), 5.f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_ValuePercentage_Extremes, "NEXUS::UnitTests::NCore::FNFloatRange::ValuePercentage_Extremes", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 100.f;
	CHECK_MESSAGE(TEXT("Minimum should map to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(0.f), 0.f, 0.001f));
	CHECK_MESSAGE(TEXT("Maximum should map to 100%"), FMath::IsNearlyEqual(Range.ValuePercentage(100.f), 1.f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_ValuePercentage_Midpoint, "NEXUS::UnitTests::NCore::FNFloatRange::ValuePercentage_Midpoint", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 10.f;
	CHECK_MESSAGE(TEXT("5 of 0..10 should be 50%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.f), 0.5f, 0.001f));

	Range.Minimum = -50.f;
	Range.Maximum = 50.f;
	CHECK_MESSAGE(TEXT("Midpoint of -50..50 should map to 50%"), FMath::IsNearlyEqual(Range.ValuePercentage(0.f), 0.5f, 0.001f));
	CHECK_MESSAGE(TEXT("Quartile of -50..50 should map to 25%"), FMath::IsNearlyEqual(Range.ValuePercentage(-25.f), 0.25f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_ValuePercentage_BelowMinimum, "NEXUS::UnitTests::NCore::FNFloatRange::ValuePercentage_BelowMinimum", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 10.f;
	Range.Maximum = 20.f;
	CHECK_MESSAGE(TEXT("Value below minimum should clamp to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.f), 0.f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_ValuePercentage_AboveMaximum, "NEXUS::UnitTests::NCore::FNFloatRange::ValuePercentage_AboveMaximum", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 10.f;
	Range.Maximum = 20.f;
	CHECK_MESSAGE(TEXT("Value above maximum should clamp to 100%"), FMath::IsNearlyEqual(Range.ValuePercentage(30.f), 1.f, 0.001f));
}

N_TEST_MEDIUM(FNFloatRangeTests_ValuePercentage_DegenerateRange, "NEXUS::UnitTests::NCore::FNFloatRange::ValuePercentage_DegenerateRange", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that a zero-width range returns 0% rather than dividing by zero.
	FNFloatRange Range;
	Range.Minimum = 5.f;
	Range.Maximum = 5.f;
	CHECK_MESSAGE(TEXT("A zero-width range should map its value to 0%"), FMath::IsNearlyEqual(Range.ValuePercentage(5.f), 0.f, 0.001f));
}

N_TEST_HIGH(FNFloatRangeTests_NextValue_WithinBounds, "NEXUS::UnitTests::NCore::FNFloatRange::NextValue_WithinBounds", N_TEST_CONTEXT_ANYWHERE)
{
	// Fractional bounds are intentional: NextValue samples through FNMersenneTwister::RandRange,
	// which has a true float overload (unlike the FRandomStream-backed Random* methods).
	FNFloatRange Range;
	Range.Minimum = 0.25f;
	Range.Maximum = 0.75f;
	FNMersenneTwister Random = FNMersenneTwister(42);
	for (int32 i = 0; i < 100; ++i)
	{
		const float Value = Range.NextValue(Random);
		CHECK_MESSAGE(FString::Printf(TEXT("NextValue[%d] should be >= Minimum"), i), Value >= 0.25f - 0.001f);
		CHECK_MESSAGE(FString::Printf(TEXT("NextValue[%d] should be <= Maximum"), i), Value <= 0.75f + 0.001f);
	}
}

N_TEST_HIGH(FNFloatRangeTests_NextValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNFloatRange::NextValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 5.5f;
	Range.Maximum = 15.5f;
	FNMersenneTwister Random = FNMersenneTwister(42);
	for (int32 i = 0; i < 100; ++i)
	{
		const float Value = Range.NextValueInSubRange(Random, 0.f, 100.f);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 5.5f - 0.001f);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 15.5f + 0.001f);
	}
}

N_TEST_HIGH(FNFloatRangeTests_RandomValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNFloatRange::RandomValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	// Bounds are whole numbers: the Random* methods sample through FRandomStream::RandRange,
	// which only has an int32 overload, so fractional bounds would truncate.
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

N_TEST_MEDIUM(FNFloatRangeTests_RandomOneShotValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNFloatRange::RandomOneShotValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 10.f;
	Range.Maximum = 20.f;
	const float A = Range.RandomOneShotValueInSubRange(4321, 0.f, 100.f);
	const float B = Range.RandomOneShotValueInSubRange(4321, 0.f, 100.f);
	CHECK_MESSAGE(TEXT("Same seed should produce same sub-range OneShot value"), FMath::IsNearlyEqual(A, B));
	CHECK_MESSAGE(TEXT("Sub-range OneShot value should be >= Range.Minimum"), A >= 10.f - 0.001f);
	CHECK_MESSAGE(TEXT("Sub-range OneShot value should be <= Range.Maximum"), A <= 20.f + 0.001f);
}

N_TEST_MEDIUM(FNFloatRangeTests_RandomTrackedValue_Determinism, "NEXUS::UnitTests::NCore::FNFloatRange::RandomTrackedValue_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 0.f;
	Range.Maximum = 100.f;
	int32 SeedA = 777;
	int32 SeedB = 777;
	const float A = Range.RandomTrackedValue(SeedA);
	const float B = Range.RandomTrackedValue(SeedB);
	CHECK_MESSAGE(TEXT("Same starting seed should produce same tracked value"), FMath::IsNearlyEqual(A, B));
	CHECK_MESSAGE(TEXT("Identical starting seeds should advance to identical values"), SeedA == SeedB);
	CHECK_MESSAGE(TEXT("Tracked sampling should advance the seed"), SeedA != 777);
}

N_TEST_MEDIUM(FNFloatRangeTests_RandomTrackedValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNFloatRange::RandomTrackedValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNFloatRange Range;
	Range.Minimum = 10.f;
	Range.Maximum = 20.f;
	int32 Seed = 246;
	for (int32 i = 0; i < 100; ++i)
	{
		const float Value = Range.RandomTrackedValueInSubRange(Seed, 0.f, 15.f);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 10.f - 0.001f);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= the sub-range maximum"), i), Value <= 15.f + 0.001f);
	}
}

#endif //WITH_TESTS
