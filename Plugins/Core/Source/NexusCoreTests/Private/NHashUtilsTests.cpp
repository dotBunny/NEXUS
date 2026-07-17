// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NHashUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNHashUtilsTests_Dbj2_Determinism, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 HashA = FNHashUtils::djb2(TEXT("TestString"));
	const uint64 HashB = FNHashUtils::djb2(TEXT("TestString"));
	CHECK_EQUALS("Same input should always produce the same hash", HashA, HashB);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_DifferentInputs, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_DifferentInputs", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 HashA = FNHashUtils::djb2(TEXT("Alpha"));
	const uint64 HashB = FNHashUtils::djb2(TEXT("Beta"));
	CHECK_MESSAGE(TEXT("Different strings should produce different hashes"), HashA != HashB);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_EmptyString, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_EmptyString", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that an empty string produces the dbj2 initial value (5381).
	const uint64 Hash = FNHashUtils::djb2(TEXT(""));
	CHECK_EQUALS("Empty string should produce the dbj2 initial seed value", Hash, 5381ull);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_CaseSensitive, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_CaseSensitive", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 Lower = FNHashUtils::djb2(TEXT("hello"));
	const uint64 Upper = FNHashUtils::djb2(TEXT("HELLO"));
	CHECK_MESSAGE(TEXT("dbj2 should be case-sensitive"), Lower != Upper);
}

N_TEST_MEDIUM(FNHashUtilsTests_Dbj2_SingleChar, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_SingleChar", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 HashA = FNHashUtils::djb2(TEXT("A"));
	const uint64 HashB = FNHashUtils::djb2(TEXT("B"));
	CHECK_MESSAGE(TEXT("Single-character strings should hash differently"), HashA != HashB);
	CHECK_MESSAGE(TEXT("Single-character hash should not be the initial seed"), HashA != 5381ull);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_KnownValue, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_KnownValue", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that ASCII input produces the canonical byte-wise djb2 value, pinning
	// backward compatibility with hashes computed before the UTF-8 encoding pass.
	const uint64 Hash = FNHashUtils::djb2(TEXT("hello"));
	CHECK_EQUALS("ASCII input should match the canonical djb2 value", Hash, 210714636441ull);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_NonAsciiDistinct, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_NonAsciiDistinct", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that characters sharing the same low byte hash differently: U+03A9 (Greek
	// capital omega) and U+00A9 (copyright sign) collide when TCHARs are truncated to one byte.
	const uint64 HashOmega = FNHashUtils::djb2(TEXT("\u03A9"));
	const uint64 HashCopyright = FNHashUtils::djb2(TEXT("\u00A9"));
	CHECK_MESSAGE(TEXT("Characters sharing a low byte should produce different hashes"), HashOmega != HashCopyright);
}

#endif //WITH_TESTS
