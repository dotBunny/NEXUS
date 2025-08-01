﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include <random>
#include "GenericPlatform/GenericPlatformMath.h"
#include "NSeedGenerator.h"

// TODO: Create non-distributed number versions

/**
 * Mersenne Twister based FRandomStream-like API with some extras!
 * Implements the <code>std::mt19937_64</code> engine to produce high quality <code>uint64</code> random numbers.
 */
class NEXUSCORE_API FNMersenneTwister
{
public:
	/**
	 * Construct a new <code>FMersenneTwister</code> with a specific seed.
	 */
	explicit FNMersenneTwister(const uint64 InSeed)
	{
		this->Initialize(InSeed);
	}

	~FNMersenneTwister() { }

	/**
	 * Set seed of the <code>FMersenneTwister</code>.
	 * @param Seed - The seed to initialize the engine with
	 **/
	void Initialize(const uint64 Seed);
	
	/**
	 * Returns a pseudo random <code>bool</code> value based on chance (<code>0</code>-<code>1</code> roll), if the result is included.
	 * @param Chance - The 0-1 percent chance of success
	 * @return a psuedo random <code>bool</code>
	 */
	bool Bias(const float Chance);

	/**
	 * Returns a pseudo random uniformly distributed <code>bool</code> value.
	 * @return a pseudo random <code>bool</code>
	 **/
	bool Bool();

	/**
	 * Returns a pseudo random <code>double</code> between <code>0</code> and <code>1</code>.
	 * @return a pseudo random <code>double</code>
	 **/
	double Double();

	/**
	 * Generate a random <code>double</code> between <code>minimum</code> and <code>maximum</code>.
	 * @param MinimumValue - The lowest possible value
	 * @param MaximumValue - The highest possible value
	 * @return a pseudo random <code>double</code>
	 **/
	float DoubleRange(const double MinimumValue = MIN_dbl, const double MaximumValue = MAX_dbl);
	FORCEINLINE float RandRange(const double MinimumValue = MIN_flt, const double MaximumValue = MAX_flt)
	{
		return DoubleRange(MinimumValue, MaximumValue);
	}

	/**
	 * Returns a pseudo random <code>float</code> between <code>0</code> and <code>1</code>.
	 * @return a pseudo random <code>float</code>
	 **/
	float Float();

	/**
	 * Generate a random <code>float</code> between <code>minimum</code> and <code>maximum</code>.
	 * @param MinimumValue - The lowest possible value
	 * @param MaximumValue - The highest possible value
	 * @return a pseudo random <code>float</code>
	 **/
	float FloatRange(const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt);
	FORCEINLINE float RandRange(const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt)
	{
		return FloatRange(MinimumValue, MaximumValue);
	}

	/**
	* Generate a pseudo random <code>integer</code> between <code>minimum</code> and <code>maximum</code>.
	* @param MinimumValue - The lowest possible value
	* @param MaximumValue - The highest possible value
	* @return a pseudo random <code>integer</code>
	**/
	int IntegerRange(const int MinimumValue = MIN_int32, const int MaximumValue = MAX_int32);
	FORCEINLINE float RandRange(const int MinimumValue = MIN_int32, const int MaximumValue = MAX_int32)
	{
		return IntegerRange(MinimumValue, MaximumValue);
	}

	/**
	* Generate a pseudo random <code>unsigned integer</code> between <code>minimum</code> and <code>maximum</code>.
	* @param MinimumValue - The lowest possible value
	* @param MaximumValue - The highest possible value
	* @return a pseudo random <code>unsigned integer</code>
	**/
	uint32 UnsignedIntegerRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32);
	FORCEINLINE float RandRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32)
	{
		return UnsignedIntegerRange(MinimumValue, MaximumValue);
	}

	
	/**
	 * Generate a pseudo random normalized <code>FVector</code>.
	 * @return A normalized random <code>FVector</code>.
	 */
	// TODO: Can this replace VRand? We should profile performance!
	FVector VectorNormalized(); 

	/**
	 * Generate a pseudo random normalized <code>FVector</code>.
	 * @return A normalized random <code>FVector</code>.
	 */
	FORCEINLINE FVector VRand()
	{
		FVector Result;
		FVector::FReal L;
		do
		{
			// Check random vectors in the unit sphere so result is statistically uniform.
			Result.X = Float() * 2.f - 1.f;
			Result.Y = Float() * 2.f - 1.f;
			Result.Z = Float() * 2.f - 1.f;
			L = Result.SizeSquared();
		}
		while(L > 1.0f || L < UE_KINDA_SMALL_NUMBER);
		return Result * (1.0f / FGenericPlatformMath::Sqrt(L));
	}
	
	/**
	 * Returns the number of times the <code>FMersenneTwister</code> has been called since the seed has been set.
	 * @return the number of times the <code>FMersenneTwister</code> has been called
	 **/
	uint32 GetCallCounter() const
	{
		return this->CallCounter;
	}

	/**
	 * Returns the seed that was last set.
	 * @return the last set seed
	 **/
	uint64 GetInitialSeed() const
	{
		return this->InitialSeed;
	}

	/**
	 * Returns the seed that was last set as a hexadecimal <code>FString</code>.
	 * @return the last set seed
	 */
	FString GetSeedAsString() const
	{
		return FNSeedGenerator::HexFromSeed(this->InitialSeed);
	}

	/**
	 *	Reset the <code>FMersenneTwister</code> to the initial seed.
	 */
	void Reset()
	{
		Initialize(this->InitialSeed);
	}

private:

	/**
	 * Bernoulli distribution probability function
	 */
	std::bernoulli_distribution BooleanDistribution{0.5};

	/**
	 * The number of times the Mersenne Twister has been called since the seed has been set.
	 */
	uint32 CallCounter; 

	/**
 	*  Single instance of the 64-bit Mersenne Twister pseudo random engine
 	*/
	// TODO: mutable?
	std::mt19937_64 Engine;

	/**
	 * Real distribution probability function
	 */
	std::uniform_real_distribution<float> FloatRangeDistribution;

	/**
	 * Real distribution probability function
	 */
	std::uniform_real_distribution<double> DoubleRangeDistribution;

	/**
	 * Uniform distribution probability function
	 */
	std::uniform_int_distribution<int> IntegerRangeDistribution;

	/**
	 * Uniform unsigned distribution probability function
	 */
	std::uniform_int_distribution<uint32> UnsignedIntegerRangeDistribution;

	/**
	 * Last seed set on the Mersenne Twister
	 */
	uint64 InitialSeed;

	/***
	 * Returns the reference to the <code>FMersenneTwister</code> engine.
	 * @return a reference to the class
	 */
	std::mt19937_64& Get()
	{
		return this->Engine;
	}
	
	FNMersenneTwister(FNMersenneTwister const&) = delete;

	/**
	 * Overloaded Assignment
	 * @return the Mersenne Twister
	 */
	FNMersenneTwister& operator=(FNMersenneTwister const&) = delete;
};
