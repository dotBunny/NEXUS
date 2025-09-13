// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include <random>
#include "GenericPlatform/GenericPlatformMath.h"
#include "NSeedGenerator.h"

// TODO: Create non-distributed number versions

/**
 * Mersenne Twister based FRandomStream-like API with some extras!
 * Implements the std::mt19937_64 engine to produce high quality uint64 random numbers.
 */
class NEXUSCORE_API FNMersenneTwister
{
public:
	/**
	 * Construct a new FMersenneTwister with a specific seed.
	 */
	explicit FNMersenneTwister(const uint64 InSeed)
	{
		this->Initialize(InSeed);
	}

	~FNMersenneTwister() { }

	/**
	 * Set seed of the FMersenneTwister.
	 * @param Seed - The seed to initialize the engine with
	 **/
	void Initialize(const uint64 Seed);
	
	/**
	 * Returns a pseudo random bool value based on chance (0-1 roll), if the result is included.
	 * @param Chance - The 0-1 percent chance of success
	 * @return a psuedo random bool
	 */
	bool Bias(const float Chance);

	/**
	 * Returns a pseudo random uniformly distributed bool value.
	 * @return a pseudo random bool
	 **/
	bool Bool();

	/**
	 * Returns a pseudo random double between 0 and 1.
	 * @return a pseudo random double
	 **/
	double Double();

	/**
	 * Generate a random double between minimum and maximum.
	 * @param MinimumValue - The lowest possible value
	 * @param MaximumValue - The highest possible value
	 * @return a pseudo random double
	 **/
	float DoubleRange(const double MinimumValue = MIN_dbl, const double MaximumValue = MAX_dbl);
	FORCEINLINE float RandRange(const double MinimumValue = MIN_flt, const double MaximumValue = MAX_flt)
	{
		return DoubleRange(MinimumValue, MaximumValue);
	}

	/**
	 * Returns a pseudo random float between 0 and 1.
	 * @return a pseudo random float
	 **/
	float Float();

	/**
	 * Generate a random float between minimum and maximum.
	 * @param MinimumValue - The lowest possible value
	 * @param MaximumValue - The highest possible value
	 * @return a pseudo random float
	 **/
	float FloatRange(const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt);
	FORCEINLINE float RandRange(const float MinimumValue = MIN_flt, const float MaximumValue = MAX_flt)
	{
		return FloatRange(MinimumValue, MaximumValue);
	}

	/**
	* Generate a pseudo random integer between minimum and maximum.
	* @param MinimumValue - The lowest possible value
	* @param MaximumValue - The highest possible value
	* @return a pseudo random integer
	**/
	int IntegerRange(const int MinimumValue = MIN_int32, const int MaximumValue = MAX_int32);
	FORCEINLINE float RandRange(const int MinimumValue = MIN_int32, const int MaximumValue = MAX_int32)
	{
		return IntegerRange(MinimumValue, MaximumValue);
	}

	/**
	* Generate a pseudo random unsigned integer between minimum and maximum.
	* @param MinimumValue - The lowest possible value
	* @param MaximumValue - The highest possible value
	* @return a pseudo random unsigned integer
	**/
	uint32 UnsignedIntegerRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32);
	FORCEINLINE float RandRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32)
	{
		return UnsignedIntegerRange(MinimumValue, MaximumValue);
	}

	
	/**
	 * Generate a pseudo random normalized FVector.
	 * @return A normalized random FVector.
	 */
	// TODO: Can this replace VRand? We should profile performance!
	FVector VectorNormalized(); 

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
	 * Returns the number of times the FMersenneTwister has been called since the seed has been set.
	 * @return the number of times the FMersenneTwister has been called
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
	 * Returns the seed that was last set as a hexadecimal FString.
	 * @return the last set seed
	 */
	FString GetSeedAsString() const
	{
		return FNSeedGenerator::HexFromSeed(this->InitialSeed);
	}

	/**
	 *	Reset the FMersenneTwister to the initial seed.
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
	 * Returns the reference to the FMersenneTwister engine.
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
