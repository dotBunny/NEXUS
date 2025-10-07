// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include <random>
#include "GenericPlatform/GenericPlatformMath.h"
#include "NSeedGenerator.h"

/**
 * Mersenne Twister based FRandomStream-like API with some extras!
 * Implements the std::mt19937_64 engine to produce high-quality uint64 random numbers.
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
	float DoubleRange(const double MinimumValue = MIN_dbl, const double MaximumValue = MAX_dbl);
	FORCEINLINE float RandRange(const double MinimumValue = MIN_flt, const double MaximumValue = MAX_flt)
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
	int IntegerRange(const int MinimumValue = MIN_int32, const int MaximumValue = MAX_int32);
	FORCEINLINE float RandRange(const int MinimumValue = MIN_int32, const int MaximumValue = MAX_int32)
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
	FORCEINLINE float RandRange(const uint32 MinimumValue = MIN_uint32, const uint32 MaximumValue = MAX_uint32)
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
	 * Generate a pseudo random normalized FVector (0-1).
	 * @return A normalized random FVector.
	 */
	FVector VectorNormalized();

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
	 * Returns the seed that was last set as a hexadecimal FString.
	 * @return the last set seed.
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
	 * Bernoulli distribution probability function.
	 */
	std::bernoulli_distribution BooleanDistribution{0.5};

	/**
	 * The number of times the Mersenne Twister has been called since the seed has been set.
	 */
	uint32 CallCounter; 

	/**
 	*  Single instance of the 64-bit Mersenne Twister pseudo random engine.
 	*/
	std::mt19937_64 Engine;
	
	/**
	 * Maintained a real float distribution probability function, explicitly used for 0-1.
	 */
	std::uniform_real_distribution<float> PersistentFloatRangeDistribution = std::uniform_real_distribution<float>(0.0, 1.0);

	/**
	 * Maintained a real double distribution probability function, explicitly used for 0-1.
	 */
	std::uniform_real_distribution<double> PersistentDoubleRangeDistribution = std::uniform_real_distribution<double>(0.0, 1.0);

	/**
	 * Last seed set on the Mersenne Twister.
	 */
	uint64 InitialSeed;

	/***
	 * Returns the reference to the FMersenneTwister engine.
	 * @return a reference to the class.
	 */
	std::mt19937_64& Get()
	{
		return this->Engine;
	}
	
	FNMersenneTwister(FNMersenneTwister const&) = delete;

	/**
	 * Overloaded Assignment
	 * @return the Mersenne Twister.
	 */
	FNMersenneTwister& operator=(FNMersenneTwister const&) = delete;
};
