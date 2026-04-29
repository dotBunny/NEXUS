// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Bucketed integer counter that records a running tally per iteration.
 *
 * Each call to NextIteration() opens a fresh bucket; Increment() and Decrement() always
 * apply to the current (most recent) bucket. The full history is preserved in Counter,
 * allowing per-iteration values to be inspected after the fact.
 *
 * Useful for analytics that need to attribute counts to a retry pass, generation step,
 * or other discrete phase rather than a single cumulative total.
 */
struct FNIterationCounter
{
	/** Index of the current bucket; always equals Counter.Num() - 1. */
	int32 Iteration = 0;

	/** Per-iteration tallies; index 0 is the first iteration, the last entry is the current bucket. */
	TArray<int32> Counter = TArray({0});

	/** Add one to the current iteration's tally. */
	void Increment()
	{
		Counter[Iteration] = Counter[Iteration] + 1;
	}

	/** Subtract one from the current iteration's tally. */
	void Decrement()
	{
		Counter[Iteration] = Counter[Iteration] - 1;
	}

	/** Open a new bucket initialized to zero and make it the current iteration. */
	void NextIteration()
	{
		Counter.Add(0);
		Iteration++;
	}
	
	/** Sum every iteration's tally and return the cumulative total across all buckets. */
	int32 GetTotal()
	{
		int32 Total = 0;
		for (int32 i = 0; i < Counter.Num(); i++)
		{
			Total += Counter[i];
		}
		return Total;
	}
};
