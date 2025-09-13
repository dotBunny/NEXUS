// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"

/**
 * An array of integers that are proportionally weighted in entry count.
 */
struct NEXUSCORE_API FNWeightedIntegerArray
{
	/**
	 * Add a value to the array, with an optional weight/count.
	 */
	void Add(const int32 Value, const int32 Weight = 1);

	/**
	 * Empty the existing array.
	 */
	void Empty();

	/**
	 * Remove a value from the array.
	 */
	void Remove(const int32 Value);

	/**
	 * Remove a value from the array, with an optional count limit.
	 */
	void RemoveSome(const int32 Value, int32 Limit = 1);

	/**
	 * Get the next deterministic value from the array.
	 * @return Uses FNRandom::Deterministic
	 */
	int32 NextValue() const
	{
		return Data[FNRandom::Deterministic.IntegerRange(0, CachedCount)];
	};

	/**
	 * Get a random value from the array.
	 * @return Uses FNRandom::NonDeterministic
	 */
	int32 RandomValue() const
	{
		return Data[FNRandom::NonDeterministic.RandRange(0, CachedCount)];
	}

	/**
	 * Get a random value from the array creating an instance FRandomStream from the Seed.	 
	 */
	int32 RandomOneShotValue(const int Seed) const
	{
		const FRandomStream RandomStream(Seed);
		return Data[RandomStream.RandRange(0, CachedCount)];
	};

	/**
	 * Get a random value from the array creating an instance FRandomStream from the Seed;
	 * additionally settings the Seed with the mutated seed.	 
	 */
	int RandomTrackedValue(int32& Seed) const
	{
		const FRandomStream Random(Seed);
		const int RandomIndex = Random.RandRange(0, CachedCount);
		Seed = Random.GetCurrentSeed();
		return Data[RandomIndex];
	};

	/**
	 * Is there any data in the array?
	 */
	bool HasData() const
	{
		return CachedCount >= 0;
	};

private:	
	TArray<int> Data;
	int CachedCount = -1;
};
