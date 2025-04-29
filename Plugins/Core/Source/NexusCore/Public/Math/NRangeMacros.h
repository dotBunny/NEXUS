// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

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
	inline Type RandomOneShotValue(int Seed) const \
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
	inline Type RandomOneShotValueInSubRange(int Seed, Type MinimumValue, Type MaximumValue) const \
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