// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NHashUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNHashUtilsTests_Dbj2_Determinism, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_Determinism", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 HashA = FNHashUtils::dbj2(TEXT("TestString"));
	const uint64 HashB = FNHashUtils::dbj2(TEXT("TestString"));
	CHECK_EQUALS("Same input should always produce the same hash", HashA, HashB);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_DifferentInputs, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_DifferentInputs", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 HashA = FNHashUtils::dbj2(TEXT("Alpha"));
	const uint64 HashB = FNHashUtils::dbj2(TEXT("Beta"));
	CHECK_MESSAGE(TEXT("Different strings should produce different hashes"), HashA != HashB);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_EmptyString, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_EmptyString", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that an empty string produces the dbj2 initial value (5381).
	const uint64 Hash = FNHashUtils::dbj2(TEXT(""));
	CHECK_EQUALS("Empty string should produce the dbj2 initial seed value", Hash, 5381ull);
}

N_TEST_HIGH(FNHashUtilsTests_Dbj2_CaseSensitive, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_CaseSensitive", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 Lower = FNHashUtils::dbj2(TEXT("hello"));
	const uint64 Upper = FNHashUtils::dbj2(TEXT("HELLO"));
	CHECK_MESSAGE(TEXT("dbj2 should be case-sensitive"), Lower != Upper);
}

N_TEST_MEDIUM(FNHashUtilsTests_Dbj2_SingleChar, "NEXUS::UnitTests::NCore::FNHashUtils::Dbj2_SingleChar", N_TEST_CONTEXT_ANYWHERE)
{
	const uint64 HashA = FNHashUtils::dbj2(TEXT("A"));
	const uint64 HashB = FNHashUtils::dbj2(TEXT("B"));
	CHECK_MESSAGE(TEXT("Single-character strings should hash differently"), HashA != HashB);
	CHECK_MESSAGE(TEXT("Single-character hash should not be the initial seed"), HashA != 5381ull);
}

#endif //WITH_TESTS
