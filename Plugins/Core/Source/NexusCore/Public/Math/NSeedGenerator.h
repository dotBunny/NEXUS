// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class NEXUSCORE_API FNSeedGenerator
{
public:

	/**
	 * Determine if the provided seed is <i>valid</i>.
	 * @param InHexSeed - The FString to check if it is a valid
	 * @return an indicator of the seeds validity
	 */
	static bool IsValidHexSeed(const FString& InHexSeed);

	/**
	 * Returns a <i>valid</i> pseudo random seed to be used with FNMersenneTwister.
	 * @return a numeric seed
	 */
	static uint64 RandomSeed();

	/**
	 * Returns a <i>valid</i> numeric seed to be used with the FNMersenneTwister.
	 * @param InSeed - The desired FString to parse
	 * @return a numeric seed
	 */
	static uint64 SeedFromText(const FString& InSeed);

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
	 * Returns the integer value of provided hexadecimal character.
	 * @param Char - The TCHAR to parse into it's numeric value
	 * @return the parsed value
	 */
	static int32 HexToInteger(const TCHAR Char);

	/**
	 * Removes all <i>non-valid</i> characters from the provided seed.
	 * @param InHexSeed - The FString input seed
	 * @return a cleaned up version of the provided seed
	 */
	static FString SanitizeHexSeed(const FString& InHexSeed);
};
