// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * A collection of random number generators.
 */
class NEXUSCORE_API FNRandom
{
public:
	/**
	 * Access the shared non-deterministic random number generator.
	 * @return A reference to the framework-wide FRandomStream seeded from the current wall-clock millisecond.
	 * @note Not thread-safe; this returns a reference to a single shared FRandomStream. FRandomStream is not internally
	 *       synchronized, so concurrent callers race on its seed. Only call from the Game-thread, or guard access externally.
	 */
	FORCEINLINE static FRandomStream& GetNonDeterministic()
	{
		static FRandomStream NonDeterministic(static_cast<int32>(FPlatformTime::Cycles64() ^ FDateTime::UtcNow().GetTicks()));
		return NonDeterministic;
	}
};