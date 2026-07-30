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
	FNMersenneTwister Random = FNMersenneTwister(42);
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.NextValue(Random);
		CHECK_MESSAGE(FString::Printf(TEXT("NextValue[%d] should be >= Minimum"), i), Value >= 0.25 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("NextValue[%d] should be <= Maximum"), i), Value <= 0.75 + 0.001);
	}
}

N_TEST_HIGH(FNDoubleRangeTests_NextValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNDoubleRange::NextValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	FNDoubleRange Range;
	Range.Minimum = 5.5;
	Range.Maximum = 15.5;
	FNMersenneTwister Random = FNMersenneTwister(42);
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.NextValueInSubRange(Random, 0.0, 100.0);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 5.5 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 15.5 + 0.001);
	}
}

N_TEST_HIGH(FNDoubleRangeTests_RandomValueInSubRange_Clamping, "NEXUS::UnitTests::NCore::FNDoubleRange::RandomValueInSubRange_Clamping", N_TEST_CONTEXT_ANYWHERE)
{
	// Fractional bounds are intentional: the Random* methods dispatch through FNRangeSampler, which routes
	// double ranges to FRandomStream::FRandRange rather than the int32-only RandRange.
	FNDoubleRange Range;
	Range.Minimum = 5.5;
	Range.Maximum = 15.5;
	for (int32 i = 0; i < 100; ++i)
	{
		const double Value = Range.RandomValueInSubRange(0.0, 100.0);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be >= Range.Minimum"), i), Value >= 5.5 - 0.001);
		CHECK_MESSAGE(FString::Printf(TEXT("SubRange value[%d] should be <= Range.Maximum"), i), Value <= 15.5 + 0.001);
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

N_TEST_MEDIUM(FNDoubleRangeTests_Sampling_ReturnsFractionalValues, "NEXUS::UnitTests::NCore::FNDoubleRange::Sampling_ReturnsFractionalValues", N_TEST_CONTEXT_ANYWHERE)
{
	// Guards every double sampling method against silently returning whole numbers. FRandomStream exposes
	// RandRange(int32, int32) alongside the separately-named FRandRange, so any sample that binds to the
	// integral overload narrows both bounds and lands exactly on 0 or 1. Every other test in this file passes
	// under that truncation -- they assert determinism and bounds only, both of which a truncating
	// implementation satisfies -- so this is the only thing that would catch it.
	FNDoubleRange Range;
	Range.Minimum = 0.0;
	Range.Maximum = 1.0;

	bool bNextFractional = false;
	bool bNextInSubRangeFractional = false;
	bool bRandomValueFractional = false;
	bool bRandomValueInSubRangeFractional = false;
	bool bOneShotFractional = false;
	bool bOneShotStreamFractional = false;
	bool bOneShotInSubRangeFractional = false;
	bool bTrackedFractional = false;
	bool bTrackedInSubRangeFractional = false;
	bool bTrackedStreamInSubRangeFractional = false;

	FNMersenneTwister Twister = FNMersenneTwister(42);
	FRandomStream OneShotStream(24680);
	FRandomStream TrackedStream(13579);
	int32 TrackedSeed = 8675309;
	int32 TrackedSubRangeSeed = 1234567;

	for (int32 i = 0; i < 100; ++i)
	{
		if (!FMath::IsNearlyZero(FMath::Frac(Range.NextValue(Twister)))) { bNextFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.NextValueInSubRange(Twister, 0.0, 1.0)))) { bNextInSubRangeFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomValue()))) { bRandomValueFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomValueInSubRange(0.0, 1.0)))) { bRandomValueInSubRangeFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomOneShotValue(i)))) { bOneShotFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomOneShotValue(OneShotStream)))) { bOneShotStreamFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomOneShotValueInSubRange(i, 0.0, 1.0)))) { bOneShotInSubRangeFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomTrackedValue(TrackedSeed)))) { bTrackedFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomTrackedValueInSubRange(TrackedSubRangeSeed, 0.0, 1.0)))) { bTrackedInSubRangeFractional = true; }
		if (!FMath::IsNearlyZero(FMath::Frac(Range.RandomTrackedValueInSubRange(TrackedStream, 0.0, 1.0)))) { bTrackedStreamInSubRangeFractional = true; }
	}

	CHECK_MESSAGE(TEXT("NextValue should produce fractional values over a 0..1 range, not just 0 or 1"), bNextFractional);
	CHECK_MESSAGE(TEXT("NextValueInSubRange should produce fractional values over a 0..1 range, not just 0 or 1"), bNextInSubRangeFractional);
	CHECK_MESSAGE(TEXT("RandomValue should produce fractional values over a 0..1 range, not just 0 or 1"), bRandomValueFractional);
	CHECK_MESSAGE(TEXT("RandomValueInSubRange should produce fractional values over a 0..1 range, not just 0 or 1"), bRandomValueInSubRangeFractional);
	CHECK_MESSAGE(TEXT("RandomOneShotValue(Seed) should produce fractional values over a 0..1 range, not just 0 or 1"), bOneShotFractional);
	CHECK_MESSAGE(TEXT("RandomOneShotValue(Stream) should produce fractional values over a 0..1 range, not just 0 or 1"), bOneShotStreamFractional);
	CHECK_MESSAGE(TEXT("RandomOneShotValueInSubRange should produce fractional values over a 0..1 range, not just 0 or 1"), bOneShotInSubRangeFractional);
	CHECK_MESSAGE(TEXT("RandomTrackedValue should produce fractional values over a 0..1 range, not just 0 or 1"), bTrackedFractional);
	CHECK_MESSAGE(TEXT("RandomTrackedValueInSubRange(Seed) should produce fractional values over a 0..1 range, not just 0 or 1"), bTrackedInSubRangeFractional);
	CHECK_MESSAGE(TEXT("RandomTrackedValueInSubRange(Stream) should produce fractional values over a 0..1 range, not just 0 or 1"), bTrackedStreamInSubRangeFractional);
}

#endif //WITH_TESTS
