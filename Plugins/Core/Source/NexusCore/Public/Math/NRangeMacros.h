// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Math/RandomStream.h"
#include "Templates/IsFloatingPoint.h"

/**
 * Routes a range sample to the FRandomStream sampler that matches the range's scalar type.
 *
 * FRandomStream splits its samplers by name rather than by overload: RandRange takes int32 only, and the
 * floating-point equivalent is separately named FRandRange. Handing a double or float to RandRange therefore
 * compiles silently but narrows both bounds to int32, so every sample lands on a whole number -- and bounds
 * outside int32's range make the conversion undefined. N_RANGE_BASE dispatches through here so each range type
 * picks the correct sampler.
 * @note The floating-point path is half-open [MinimumValue, MaximumValue) because FRandRange builds on FRand;
 *       the integral path remains fully inclusive.
 */
struct FNRangeSampler
{
	/** Sample RandomStream between MinimumValue and MaximumValue using the sampler appropriate for Type. @return a pseudo random Type. */
	template <typename Type>
	FORCEINLINE static Type Sample(const FRandomStream& RandomStream, const Type MinimumValue, const Type MaximumValue)
	{
		if constexpr (TIsFloatingPoint<Type>::Value)
		{
			return static_cast<Type>(RandomStream.FRandRange(MinimumValue, MaximumValue));
		}
		else
		{
			return RandomStream.RandRange(MinimumValue, MaximumValue);
		}
	}
};

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
 * The Next* methods sample through FNMersenneTwister, which overloads RandRange per scalar type. The Random*
 * methods sample through FRandomStream, which does not, so they route via FNRangeSampler::Sample.
 *
 * @param Type The underlying scalar type of the range (double, float, int32, ...).
 */
#define N_RANGE_BASE(Type) \
	inline Type NextValue(FNMersenneTwister& Twister) const \
	{ \
		return Twister.RandRange(Minimum, Maximum); \
	} \
	inline Type NextValueInSubRange(FNMersenneTwister& Twister, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		return Twister.RandRange(MinimumValue, MaximumValue); \
	} \
	inline Type PercentageValue(const float Percentage) const \
	{ \
		return Maximum * Percentage + Minimum * (1.f - Percentage); \
	} \
	inline Type RandomValue() const \
	{ \
		return FNRangeSampler::Sample<Type>(FNRandom::GetNonDeterministic(), Minimum, Maximum); \
	} \
	inline Type RandomOneShotValue(FRandomStream& RandomStream) const \
	{ \
		return FNRangeSampler::Sample<Type>(RandomStream, Minimum, Maximum); \
	} \
	inline Type RandomOneShotValue(int32 Seed) const \
	{ \
		const FRandomStream RandomStream(Seed); \
		return FNRangeSampler::Sample<Type>(RandomStream, Minimum, Maximum); \
	} \
	inline Type RandomTrackedValue(int& Seed) const \
	{ \
		FRandomStream RandomStream(Seed); \
		const Type ReturnValue = FNRangeSampler::Sample<Type>(RandomStream, Minimum, Maximum); \
		Seed = RandomStream.GetCurrentSeed(); \
		return ReturnValue; \
	} \
	inline Type RandomValueInSubRange(Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		return FNRangeSampler::Sample<Type>(FNRandom::GetNonDeterministic(), MinimumValue, MaximumValue); \
	} \
	inline Type RandomTrackedValueInSubRange(FRandomStream& RandomStream, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		return FNRangeSampler::Sample<Type>(RandomStream, MinimumValue, MaximumValue); \
	} \
	inline Type RandomOneShotValueInSubRange(int32 Seed, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		const FRandomStream RandomStream(Seed); \
		return FNRangeSampler::Sample<Type>(RandomStream, MinimumValue, MaximumValue); \
	} \
	inline Type RandomTrackedValueInSubRange(int& Seed, Type MinimumValue, Type MaximumValue) const \
	{ \
		if (MinimumValue < Minimum) { MinimumValue = Minimum; } \
		if (MaximumValue > Maximum) { MaximumValue = Maximum; } \
		FRandomStream RandomStream(Seed); \
		const Type ReturnValue = FNRangeSampler::Sample<Type>(RandomStream, MinimumValue, MaximumValue); \
		Seed = RandomStream.GetCurrentSeed(); \
		return ReturnValue; \
	} \
	inline float ValuePercentage(const Type Value) const \
	{ \
		if (Value < Minimum) { return 0.f; } \
		if (Value > Maximum) { return 1.f; } \
		if (Maximum == Minimum) { return 0.f; } \
		const double Range = static_cast<double>(Maximum) - static_cast<double>(Minimum); \
		return static_cast<float>((static_cast<double>(Value) - static_cast<double>(Minimum)) / Range); \
	}
