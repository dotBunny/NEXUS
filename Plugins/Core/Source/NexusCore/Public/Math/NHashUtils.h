// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * A collection of non-cryptographic hashing utilities.
 */
class NEXUSCORE_API FNHashUtils
{
public:
	/**
	 * Produces a 64-bit hash of the supplied string using Dan Bernstein's djb2 algorithm.
	 * @param InString The string to hash.
	 * @return The 64-bit djb2 hash of InString.
	 * @note Not cryptographically secure; suitable for hash tables and non-security-sensitive identity checks.
	 */
	static uint64 dbj2(const FString& InString);
};