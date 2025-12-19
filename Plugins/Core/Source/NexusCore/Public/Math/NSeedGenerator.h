// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class NEXUSCORE_API FNSeedGenerator
{
public:

	/**
	 * Determine if the provided seed is valid.
	 * @param InHexSeed - The FString to check if it is valid
	 * @return an indicator of the seeds' validity
	 */
	static bool IsValidHexSeed(const FString& InHexSeed);

	/**
	 * Returns a valid pseudo random seed to be used with FNMersenneTwister.
	 * @return a numeric seed
	 */
	static uint64 RandomSeed();

	/**
	 * Returns a valid pseudo random friendly seed.
	 * @return A friendly seed string
	 */
	static FString RandomFriendlySeed();

	/**
	 * Returns a <i>valid</i> numeric seed to be used with the FNMersenneTwister.
	 * @param InSeed - The desired FString to parse
	 * @return a numeric seed
	 */
	static uint64 SeedFromString(const FString& InSeed);

	static uint64 SeedFromFriendlySeed(const FString& InSeed);
	
	/**
	 * Returns a <i>valid</i> numeric seed to be used with the FNMersenneTwister.
	 * @param InHexSeed - The desired hexadecimal FString to parse
	 * @return a numeric seed
	 */
	static uint64 SeedFromHex(const FString& InHexSeed);

	/**
	 * Returns the hexadecimal FString of the provided value.
	 * @param Seed - The desired numeric value to convert
	 * @return a hexadecimal formatted seed
	 */
	static FString HexFromSeed(const uint64 Seed);

private:

	/**
	 * Returns the integer value of the provided hexadecimal character.
	 * @param Char - The character to parse into it's numeric value
	 * @return the parsed value
	 */
	static int32 HexToInteger(const TCHAR Char);

	/**
	 * Removes all non-valid characters from the provided seed.
	 * @param InHexSeed - The FString input seed
	 * @return a cleaned-up version of the provided seed
	 */
	static FString SanitizeHexSeed(const FString& InHexSeed);
	
};
