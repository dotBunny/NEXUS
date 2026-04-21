// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Implements the common member API shared by the NEXUS range structs.
 *
 * Expands to a family of inline methods that sample values between the struct's Minimum and Maximum
 * fields using various random sources (deterministic, non-deterministic, seeded, or externally-tracked).
 * Call sites on FNDoubleRange/FNFloatRange/FNIntegerRange can therefore share a single author-time API.
 *
 * Generated methods:
 *  - NextValue() — deterministic sample from the full range.
 *  - NextValueInSubRange(Min, Max) — deterministic sample from a clamped sub-range.
 *  - PercentageValue(0..1) — linear interpolation between Minimum and Maximum.
 *  - RandomValue() — non-deterministic sample from the full range.
 *  - RandomOneShotValue(Stream|Seed) — sample once from an ad-hoc stream.
 *  - RandomTrackedValue(Seed&) — sample while advancing the supplied seed.
 *  - RandomValueInSubRange / RandomOneShotValueInSubRange / RandomTrackedValueInSubRange — sub-range variants.
 *  - RandomTrackedValueInSubRange(Stream&, Min, Max) — sub-range sample from an externally-owned stream.
 *  - ValuePercentage(Value) — inverse of PercentageValue; returns Value's [0..1] position within the range.
 *
 * @param Type The underlying scalar type of the range (double, float, int32, ...).
 */
#define N_IMPLEMENT_RANGE(Type) \
	inline Type NextValue() const \
	{ \
		return FNRandom::Deterministic.RandRange(Minimum, Maximum); \
	} \
	inline Type NextValueInSubRange(Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		return FNRandom::Deterministic.RandRange(MinimumValue, MaximumValue); \
	} \
	inline Type PercentageValue(const float Percentage) const \
	{ \
		return Maximum * Percentage + Minimum * (1.f - Percentage); \
	} \
	inline Type RandomValue() const \
	{ \
		return FNRandom::NonDeterministic.RandRange(Minimum, Maximum); \
	} \
	inline Type RandomOneShotValue(FRandomStream& RandomStream) const \
	{ \
		return RandomStream.RandRange(Minimum, Maximum); \
	} \
	inline Type RandomOneShotValue(int32 Seed) const \
	{ \
		const FRandomStream RandomStream(Seed); \
		return RandomStream.RandRange(Minimum, Maximum); \
	} \
	inline Type RandomTrackedValue(int& Seed) const \
	{ \
		const FRandomStream RandomStream(Seed); \
		const Type ReturnValue = RandomStream.RandRange(Minimum, Maximum); \
		Seed = RandomStream.GetCurrentSeed(); \
		return ReturnValue; \
	} \
	inline Type RandomValueInSubRange(Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		return FNRandom::NonDeterministic.RandRange(MinimumValue, MaximumValue); \
	} \
	inline Type RandonTrackedValueInSubRange(FRandomStream& RandomStream, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		return RandomStream.RandRange(MinimumValue, MaximumValue); \
	} \
	inline Type RandomOneShotValueInSubRange(int32 Seed, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		const FRandomStream RandomStream(Seed); \
		return RandomStream.RandRange(MinimumValue, MaximumValue); \
	} \
	inline Type RandomTrackedValueInSubRange(int& Seed, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		const FRandomStream RandomStream(Seed); \
		const Type ReturnValue = RandomStream.RandRange(MinimumValue, MaximumValue); \
		Seed = RandomStream.GetCurrentSeed(); \
		return ReturnValue; \
	} \
	inline float ValuePercentage(const Type Value) const \
	{ \
		if (Value < Minimum) { return 0.f; } \
		if (Value > Maximum) { return 1.f; } \
		return (Value - Minimum) / (Maximum - Minimum); \
	}