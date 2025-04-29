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
	 * A deterministic random number generator.
	 * @remark It is SUPER important that values are called from this in a deterministic order.
	 */
	static FNMersenneTwister Deterministic;
	/**
	 * A non-deterministic random number generator that can be used at anytime.
	 */
	static FRandomStream NonDeterministic;
};