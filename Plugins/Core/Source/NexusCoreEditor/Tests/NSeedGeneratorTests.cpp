// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NSeedGenerator.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNSeedGeneratorTests_IsValidHexSeed_ValidSeeds, "NEXUS::UnitTests::NCore::FNSeedGenerator::IsValidHexSeed_ValidSeeds", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Pure hex digits should be valid"), FNSeedGenerator::IsValidHexSeed(TEXT("0123456789ABCDEF")));
	CHECK_MESSAGE(TEXT("Lowercase hex digits should be valid"), FNSeedGenerator::IsValidHexSeed(TEXT("abcdef")));
	CHECK_MESSAGE(TEXT("Single digit should be valid"), FNSeedGenerator::IsValidHexSeed(TEXT("F")));
}

N_TEST_HIGH(FNSeedGeneratorTests_IsValidHexSeed_InvalidSeeds, "NEXUS::UnitTests::NCore::FNSeedGenerator::IsValidHexSeed_InvalidSeeds", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("Empty string should be invalid"), FNSeedGenerator::IsValidHexSeed(TEXT("")));
	CHECK_FALSE_MESSAGE(TEXT("Non-hex characters should be invalid"), FNSeedGenerator::IsValidHexSeed(TEXT("GHIJK")));
	CHECK_MESSAGE(TEXT("Spaces should be removed, and thus valid"), FNSeedGenerator::IsValidHexSeed(TEXT("AB CD")));
}

N_TEST_HIGH(FNSeedGeneratorTests_HexFromSeed_KnownValues, "NEXUS::UnitTests::NCore::FNSeedGenerator::HexFromSeed_KnownValues", N_TEST_CONTEXT_ANYWHERE)
{
	const FString HexZero = FNSeedGenerator::HexFromSeed(0);
	CHECK_MESSAGE(TEXT("Seed 0 should produce a valid hex string"), FNSeedGenerator::IsValidHexSeed(HexZero));

	const FString Hex255 = FNSeedGenerator::HexFromSeed(255);
	CHECK_MESSAGE(TEXT("Seed 255 should produce a valid hex string"), FNSeedGenerator::IsValidHexSeed(Hex255));
}

N_TEST_HIGH(FNSeedGeneratorTests_SeedFromHex_RoundTrip, "NEXUS::UnitTests::NCore::FNSeedGenerator::SeedFromHex_RoundTrip", N_TEST_CONTEXT_ANYWHERE)
{
	constexpr uint64 OriginalSeed = 123456789ull;
	const FString Hex = FNSeedGenerator::HexFromSeed(OriginalSeed);
	const uint64 Recovered = FNSeedGenerator::SeedFromHex(Hex);
	CHECK_EQUALS("HexFromSeed -> SeedFromHex round-trip should recover the original seed", Recovered, OriginalSeed);
}

N_TEST_HIGH(FNSeedGeneratorTests_SeedFromString_NonEmpty, "NEXUS::UnitTests::NCore::FNSeedGenerator::SeedFromString_NonEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 SeedA = FNSeedGenerator::SeedFromString(TEXT("Hello"));
	const uint64 SeedB = FNSeedGenerator::SeedFromString(TEXT("World"));
	CHECK_MESSAGE(TEXT("Different strings should produce different seeds"), SeedA != SeedB);
}

N_TEST_HIGH(FNSeedGeneratorTests_SeedFromString_Determinism, "NEXUS::UnitTests::NCore::FNSeedGenerator::SeedFromString_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 SeedA = FNSeedGenerator::SeedFromString(TEXT("Deterministic"));
	const uint64 SeedB = FNSeedGenerator::SeedFromString(TEXT("Deterministic"));
	CHECK_EQUALS("Same string should always produce the same seed", SeedA, SeedB);
}

N_TEST_HIGH(FNSeedGeneratorTests_RandomSeed_NonZero, "NEXUS::UnitTests::NCore::FNSeedGenerator::RandomSeed_NonZero", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 Seed = FNSeedGenerator::RandomSeed();
	CHECK_MESSAGE(TEXT("RandomSeed should return a non-zero value"), Seed != 0);
}

N_TEST_HIGH(FNSeedGeneratorTests_RandomFriendlySeed_NotEmpty, "NEXUS::UnitTests::NCore::FNSeedGenerator::RandomFriendlySeed_NotEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	const FString Seed = FNSeedGenerator::RandomFriendlySeed();
	CHECK_MESSAGE(TEXT("RandomFriendlySeed should return a non-empty string"), Seed.Len() > 0);
}

N_TEST_MEDIUM(FNSeedGeneratorTests_SeedFromFriendlySeed_Determinism, "NEXUS::UnitTests::NCore::FNSeedGenerator::SeedFromFriendlySeed_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	const FString Friendly = FNSeedGenerator::RandomFriendlySeed();
	const uint64 SeedA = FNSeedGenerator::SeedFromFriendlySeed(Friendly);
	const uint64 SeedB = FNSeedGenerator::SeedFromFriendlySeed(Friendly);
	CHECK_EQUALS("Same friendly seed should always produce the same numeric seed", SeedA, SeedB);
}

#endif //WITH_TESTS
