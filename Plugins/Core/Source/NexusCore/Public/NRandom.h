// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Math/NMersenneTwister.h"

/**
 * A collection of random number generators.
 */
class NEXUSCORE_API FNRandom
{
public:
	/**
	 * Access the shared deterministic random number generator.
	 * @return A reference to the framework-wide Mersenne Twister seeded from the "NEXUS" string.
	 * @note Values must be drawn from this generator in a deterministic order across runs; any divergence in call order will desync downstream consumers.
	 * @note It is on the developer to handle cross thread issues (tldr don't there's no way to ensure order).
	 */
	FORCEINLINE static FNMersenneTwister& GetDeterministic()
	{
		return Deterministic;
	}

	/**
	 * Access the shared non-deterministic random number generator.
	 * @return A reference to the framework-wide FRandomStream seeded from the current wall-clock millisecond.
	 */
	FORCEINLINE static FRandomStream& GetNonDeterministic()
	{
		return NonDeterministic;
	}
	
private:
	/**
	 * A deterministic random number generator.
	 * @note It is SUPER important that values are called from this in a deterministic order.
	 */
	static FNMersenneTwister Deterministic;
	/** A non-deterministic random number generator that can be used at any time. */
	static FRandomStream NonDeterministic;
};