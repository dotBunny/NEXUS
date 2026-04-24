// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NRandom.h"

/**
 * An array of integers that are proportionally weighted via repeated entries.
 *
 * Rather than storing explicit weight tables, each value is inserted Weight times. Picking an
 * entry then becomes a uniform random index lookup while still honoring the relative weights.
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
	 * Get the next deterministic value from the array, then remove every copy of it.
	 * @return The picked value, which will no longer appear in the array on subsequent calls. Uses FNRandom::Deterministic.
	 */
	int32 NextValueAndRemove()
	{
		const int32 ReturnValue = Data[FNRandom::Deterministic.IntegerRange(0, CachedMaxIndex)];;
		Remove(ReturnValue);
		return ReturnValue;
	}

	/**
	 * Get a random value from the array.
	 * @return Uses FNRandom::NonDeterministic
	 */
	int32 RandomValue() const
	{
		return Data[FNRandom::NonDeterministic.RandRange(0, CachedMaxIndex)];
	}

	/**
	 * Get a random value from the array, then remove every copy of it.
	 * @return The picked value, which will no longer appear in the array on subsequent calls. Uses FNRandom::NonDeterministic.
	 */
	int32 RandomValueAndRemove()
	{
		const int32 ReturnValue = Data[FNRandom::NonDeterministic.RandRange(0, CachedMaxIndex)];
		Remove(ReturnValue);
		return ReturnValue;
	}

	/** Get a random value from the array creating an instance FRandomStream from the Seed. */
	int32 RandomOneShotValue(const int Seed) const
	{
		const FRandomStream RandomStream(Seed);
		return Data[RandomStream.RandRange(0, CachedMaxIndex)];
	};

	/**
	 * Get a random value from the array creating an instance FRandomStream from the Seed, then remove every copy of it.
	 * @param Seed The seed used to construct the FRandomStream for this one-shot pick.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	int32 RandomOneShotValueAndRemove(const int Seed)
	{
		const FRandomStream RandomStream(Seed);
		const int32 ReturnValue = Data[RandomStream.RandRange(0, CachedMaxIndex)];
		Remove(ReturnValue);
		return ReturnValue;
	}

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
	 * Get a random value from the array creating an instance FRandomStream from the Seed, then remove every copy of it;
	 * additionally setting the Seed with the mutated seed.
	 * @param Seed The seed used to construct the FRandomStream; updated to the stream's mutated seed on return.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	int32 RandomTrackedValueAndRemove(int32& Seed)
	{
		const FRandomStream Random(Seed);
		const int RandomIndex = Random.RandRange(0, CachedMaxIndex);
		Seed = Random.GetCurrentSeed();
		const int32 ReturnValue = Data[RandomIndex];
		Remove(ReturnValue);
		return ReturnValue;
	}

	/**
	 * Get a random value from the array from a deterministic input.
	 * @param Twister The Mersenne Twister to query for random.
	 */
	int32 TwistedValue(FNMersenneTwister& Twister) const
	{
		const int RandomIndex = Twister.IntegerRange(0, CachedMaxIndex);
		return Data[RandomIndex];
	}
	
	/**
	 * Get a random value from the array from a deterministic input, then remove every copy of it.
	 * @param Twister The Mersenne Twister to query for random.
	 * @return The picked value, which will no longer appear in the array on subsequent calls.
	 */
	int32 TwistedValueAndRemove(FNMersenneTwister& Twister)
	{
		const int RandomIndex = Twister.IntegerRange(0, CachedMaxIndex);
		const int32 ReturnValue = Data[RandomIndex];
		Remove(ReturnValue);
		return ReturnValue;
	}

	/** Is there any data in the array? */
	bool HasData() const
	{
		return Data.Num() > 0;
	};
	
	/**
	 * Does the array currently contain at least one copy of the supplied value?
	 * @param Value The value to test for.
	 * @return True if Value is present in the array, false otherwise.
	 */
	bool HasValue(const int32 Value) const { return Values.Contains(Value); }

	/**
	 * How many entries remain in the weighted array?
	 * @return The number of entries left?
	 */
	int32 WeightedCount() const { return Data.Num(); }
	
	/**
	 * How many distinct values remain in the array, ignoring weighting?
	 * @return The number of unique values currently present.
	 */
	int32 ValueCount() const { return Values.Num(); }

private:
	TArray<int32> Values;
	TArray<int32> Data;
	int32 CachedMaxIndex = 0;
};
