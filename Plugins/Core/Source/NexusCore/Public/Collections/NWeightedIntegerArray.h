// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"

/**
 * An array of integers that are proportionally weighted via repeated entries.
 *
 * Rather than storing explicit weight tables, each value is inserted Weight times. Picking an
 * entry then becomes a uniform random index lookup while still honouring the relative weights.
 * This keeps selection fast at the cost of a larger memory footprint for heavily weighted entries.
 */
struct NEXUSCORE_API FNWeightedIntegerArray
{
	/**
	 * Add a value to the array, duplicated according to its weight.
	 * @param Value The integer value to add.
	 * @param Weight How many copies of Value to insert (the value's relative likelihood of selection).
	 */
	void Add(const int32 Value, const int32 Weight = 1);

	/** Clears all entries from the array. */
	void Empty();

	/**
	 * Removes every copy of the supplied value from the array.
	 * @param Value The value whose entries should all be removed.
	 */
	void Remove(const int32 Value);

	/**
	 * Removes up to Limit copies of the supplied value from the array.
	 * @param Value The value whose entries should be partially removed.
	 * @param Limit The maximum number of copies to remove.
	 */
	void RemoveSome(const int32 Value, int32 Limit = 1);

	/**
	 * Get the next deterministic value from the array.
	 * @return Uses FNRandom::Deterministic
	 */
	int32 NextValue() const
	{
		return Data[FNRandom::Deterministic.IntegerRange(0, CachedMaxIndex)];
	};

	/**
	 * Get a random value from the array.
	 * @return Uses FNRandom::NonDeterministic
	 */
	int32 RandomValue() const
	{
		return Data[FNRandom::NonDeterministic.RandRange(0, CachedMaxIndex)];
	}

	/** Get a random value from the array creating an instance FRandomStream from the Seed. */
	int32 RandomOneShotValue(const int Seed) const
	{
		const FRandomStream RandomStream(Seed);
		return Data[RandomStream.RandRange(0, CachedMaxIndex)];
	};

	/**
	 * Get a random value from the array creating an instance FRandomStream from the Seed;
	 * additionally settings the Seed with the mutated seed.	 
	 */
	int32 RandomTrackedValue(int32& Seed) const
	{
		const FRandomStream Random(Seed);
		const int RandomIndex = Random.RandRange(0, CachedMaxIndex);
		Seed = Random.GetCurrentSeed();
		return Data[RandomIndex];
	};

	/**
	 * Get a random value from the array from a deterministic input.
	 * @param Twister The Mersenne Twister to query for random.
	 */
	int32 TwistedValue(FNMersenneTwister& Twister) const
	{
		const int RandomIndex = Twister.IntegerRange(0, CachedMaxIndex);
		return Data[RandomIndex];
	}

	/** Is there any data in the array? */
	bool HasData() const
	{
		return Data.Num() > 0;
	};

	/**
	 * How many entries remain in the weighted array?
	 * @return The number of entries left?
	 */
	int32 Count() const { return Data.Num(); }

private:	
	TArray<int32> Data;
	int32 CachedMaxIndex = 0;
};
