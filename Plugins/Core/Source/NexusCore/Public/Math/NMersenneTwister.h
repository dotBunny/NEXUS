// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include <random>
#include "GenericPlatform/GenericPlatformMath.h"
#include "NSeedGenerator.h"

/**
 * Mersenne Twister based FRandomStream-like API with some extras!
 * Implements the std::mt19937_64 engine to produce high-quality uint64 random numbers.
 * Guaranteed behavior across platforms/compilers by avoiding using std::*_distribution.
 */
class NEXUSCORE_API FNMersenneTwister
{
public:
	/** Construct a new FMersenneTwister with a specific seed. */
	explicit FNMersenneTwister(const uint64 InSeed)
	{
		this->Initialize(InSeed);
	}

	~FNMersenneTwister() = default;

	/**
	 * Set seed of the FMersenneTwister.
	 * @param Seed The seed to initialize the engine with.
	 */
	void Initialize(const uint64 Seed);
	
	/**
	 * Returns a pseudo random bool value based on chance (0-1 roll), if the result is included.
	 * @param Chance The 0-1 percent chance of success
	 * @return a pseudo random bool.
	 */
	bool Bias(const float Chance);

	/**
	 * Returns an array of pseudo random bool value based on chance (0-1 roll), if the result is included.
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param Chance The 0-1 percent chance of success
	 * @param StartIndex The index to start writing at.
	 */
	void Bias(TArray<bool>& OutArray, const int32 Count, const float Chance, const int32 StartIndex = 0);

	/**
	 * Returns a pseudo random uniformly distributed bool value.
	 * @return a pseudo random bool.
	 */
	bool Bool();

	/**
	 * Returns an array of pseudo random bool values based on a coin-flip.
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param StartIndex The index to start writing at.
	 */
	void Bool(TArray<bool>& OutArray, const int32 Count, const int32 StartIndex = 0);

	/**
	 * Returns a pseudo random double between 0 and 1.
	 * @return a pseudo random double.
	 */
	double Double();

	/**
	 * Returns an array of pseudo random double between 0 and 1.
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param StartIndex The index to start writing at.
	 */
	void Double(TArray<double>& OutArray, const int32 Count, const int32 StartIndex = 0);
	
	/**
	 * Generate a random double between minimum and maximum.
	 * @param MinimumValue The lowest possible value.
	 * @param MaximumValue The highest possible value.
	 * @return a pseudo random double.
	 */
	double DoubleRange(const double MinimumValue = MIN_dbl, const double MaximumValue = MAX_dbl);
	FORCEINLINE double RandRange(const double MinimumValue = MIN_dbl, const double MaximumValue = MAX_dbl)
	{
		return DoubleRange(MinimumValue, MaximumValue);
	}

	/**
	 * Returns an array of pseudo random double values between the MinimumValue and MaximumValue (inclusive).
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param MinimumValue The lowest possible value.
	 * @param MaximumValue The highest possible value.
	 * @param StartIndex The index to start writing at.
	 */
	void DoubleRange(TArray<double>& OutArray, const int32 Count, const double MinimumValue = MIN_dbl, const double MaximumValue = MAX_dbl, const int32 StartIndex = 0);
	
	/**
	 * Returns a pseudo random float between 0 and 1.
	 * @return a pseudo random float.
	 */
	float Float();

	/**
	 * Returns an array of pseudo random float between 0 and 1.
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param StartIndex The index to start writing at.
	 */
	void Float(TArray<float>& OutArray, const int32 Count, const int32 StartIndex = 0);

	/**
	 * Generate a random float between minimum and maximum.
	 * @param MinimumValue The lowest possible value.
	 * @param MaximumValue The highest possible value.
	 * @return a pseudo random float.
	 */
	float FloatRange(const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt);
	FORCEINLINE float RandRange(const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt)
	{
		return FloatRange(MinimumValue, MaximumValue);
	}

	/**
	 * Returns an array of pseudo random float values between the MinimumValue and MaximumValue (inclusive).
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param MinimumValue The lowest possible value.
	 * @param MaximumValue The highest possible value.
	 * @param StartIndex The index to start writing at.
	 */
	void FloatRange(TArray<float>& OutArray, const int32 Count, const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt, const int32 StartIndex = 0);

	/**
	* Generate a pseudo random integer between minimum and maximum.
	* @param MinimumValue The lowest possible value.
	* @param MaximumValue The highest possible value.
	* @return a pseudo random integer.
	*/
	int32 IntegerRange(const int32 MinimumValue = MIN_int32, const int32 MaximumValue = MAX_int32);
	FORCEINLINE int32 RandRange(const int32 MinimumValue = MIN_int32, const int32 MaximumValue = MAX_int32)
	{
		return IntegerRange(MinimumValue, MaximumValue);
	}

	/**
	 * Returns an array of pseudo random integer values between the MinimumValue and MaximumValue (inclusive).
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param MinimumValue The lowest possible value.
	 * @param MaximumValue The highest possible value.
	 * @param StartIndex The index to start writing at.
	 */
	void IntegerRange(TArray<int32>& OutArray, const int32 Count, const int32 MinimumValue = MIN_int32, const int32 MaximumValue = MAX_int32, const int32 StartIndex = 0);

	/**
	* Generate a pseudo random unsigned integer between minimum and maximum.
	* @param MinimumValue The lowest possible value.
	* @param MaximumValue The highest possible value.
	* @return a pseudo random unsigned integer.
	*/
	uint32 UnsignedIntegerRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32);
	FORCEINLINE uint32 RandRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32)
	{
		return UnsignedIntegerRange(MinimumValue, MaximumValue);
	}

	/**
	 * Returns an array of pseudo random unsigned integer values between the MinimumValue and MaximumValue (inclusive).
	 * @param OutArray A pre-allocated array to fill with the results.
	 * @param Count The number of results to generate.
	 * @param MinimumValue The lowest possible value.
	 * @param MaximumValue The highest possible value.
	 * @param StartIndex The index to start writing at.
	 */
	void UnsignedIntegerRange(TArray<uint32>& OutArray, const int32 Count,const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32, const int32 StartIndex = 0);

	/**
	 * Generates a pseudo random FVector using the provided ranges.
	 * @param MinimumRange The minimum X, Y, Z range.
	 * @param MaximumRange  The maximum X, Y, Z range.
	 * @return The random FVector.
	 */
	FVector Vector(const float MinimumRange = MIN_flt, const float MaximumRange = MAX_flt);
	
	/**
	 * Generate a pseudo random normalized FVector.
	 * @return A normalized random FVector.
	 */
	FORCEINLINE FVector VRand()
	{
		FVector Result;
		FVector::FReal L;
		do
		{
			// Check random vectors in the unit sphere so the result is statistically uniform.
			Result.X = Float() * 2.f - 1.f;
			Result.Y = Float() * 2.f - 1.f;
			Result.Z = Float() * 2.f - 1.f;
			L = Result.SizeSquared();
		}
		while(L > 1.0f || L < UE_KINDA_SMALL_NUMBER);
		return Result * (1.0f / FGenericPlatformMath::Sqrt(L));
	}
	
	/**
	 * Returns the number of times the FMersenneTwister has been called since the seed has been set.
	 * @return the number of times the FMersenneTwister has been called.
	 */
	uint32 GetCallCounter() const
	{
		return this->CallCounter;
	}

	/**
	 * Returns the seed that was last set.
	 * @return the last set seed.
	 */
	uint64 GetInitialSeed() const
	{
		return this->InitialSeed;
	}


	/**
	 * Generates a pseudo random unsigned 64-bit integer spanning the full range of uint64.
	 * @return The next uint64 drawn from the underlying engine.
	 */
	uint64 UnsignedInteger64()
	{
		this->CallCounter++;
		return this->Engine();
	}
	
	/**
	 * Returns the seed that was last set as a hexadecimal FString.
	 * @return the last set seed.
	 */
	FString GetSeedAsString() const
	{
		return FNSeedGenerator::HexFromSeed(this->InitialSeed);
	}

	/** Reset the FMersenneTwister to the initial seed. */
	void Reset()
	{
		Initialize(this->InitialSeed);
	}

private:

	/** The number of times the Mersenne Twister has been called since the seed has been set. */
	uint32 CallCounter;

	/** Single instance of the 64-bit Mersenne Twister pseudo random engine. */
	std::mt19937_64 Engine;

	/** The last seed set on the Mersenne Twister. */
	uint64 InitialSeed;

	/**
	 * Maps a raw engine draw to a float in the half-open range [0, 1).
	 * @param Bits A raw uint64 produced by the engine.
	 * @return a portable pseudo random float in [0, 1).
	 * @note Uses the top 24 bits (full float mantissa precision) so the result depends only on the
	 *       standard-defined engine output, not on the STL vendor's distribution implementation.
	 */
	static FORCEINLINE float ToUnitFloat(const uint64 Bits)
	{
		// 2^24 = 16777216
		return static_cast<float>(Bits >> 40) * (1.0f / 16777216.0f);
	}

	/**
	 * Maps a raw engine draw to a double in the half-open range [0, 1).
	 * @param Bits A raw uint64 produced by the engine.
	 * @return a portable pseudo random double in [0, 1).
	 * @note Uses the top 53 bits (full double mantissa precision) so the result depends only on the
	 *       standard-defined engine output, not on the STL vendor's distribution implementation.
	 */
	static FORCEINLINE double ToUnitDouble(const uint64 Bits)
	{
		// 2^53 = 9007199254740992
		return static_cast<double>(Bits >> 11) * (1.0 / 9007199254740992.0);
	}

	/**
	 * Draws a uniformly distributed value in the half-open range [0, Range).
	 * @param Range The exclusive upper bound, must be >= 1.
	 * @return a portable pseudo random uint64 in [0, Range).
	 * @note Uses rejection sampling to remove modulo bias; the result depends only on the engine's
	 *       (standard-defined) output, so it is identical across platforms and toolchains.
	 */
	uint64 NextBounded(const uint64 Range);

	FNMersenneTwister(FNMersenneTwister const&) = delete;

	/**
	 * Overloaded Assignment
	 * @return the Mersenne Twister.
	 */
	FNMersenneTwister& operator=(FNMersenneTwister const&) = delete;
};
