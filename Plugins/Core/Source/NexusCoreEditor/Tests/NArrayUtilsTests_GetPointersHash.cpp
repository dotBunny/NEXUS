// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArrayUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNArrayUtilsTests_GetPointersHash_EmptyArray, "NEXUS::UnitTests::NCore::FNArrayUtils::GetPointersHash_EmptyArray", N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<int32*> Elements;
	CHECK_EQUALS("An empty array should hash to zero", FNArrayUtils::GetPointersHash(Elements), 0u);
}

N_TEST_HIGH(FNArrayUtilsTests_GetPointersHash_SingleElement, "NEXUS::UnitTests::NCore::FNArrayUtils::GetPointersHash_SingleElement", N_TEST_CONTEXT_ANYWHERE)
{
	// Pins the documented contract: the result is the XOR-combination of each pointer's
	// GetTypeHash, so a single element hashes to exactly its own GetTypeHash value.
	int32 A = 1;
	const TArray<int32*> Elements = {&A};
	CHECK_EQUALS("A single-element array should hash to the pointer's GetTypeHash", FNArrayUtils::GetPointersHash(Elements), GetTypeHash(&A));
}

N_TEST_HIGH(FNArrayUtilsTests_GetPointersHash_OrderIndependence, "NEXUS::UnitTests::NCore::FNArrayUtils::GetPointersHash_OrderIndependence", N_TEST_CONTEXT_ANYWHERE)
{
	int32 A = 1;
	int32 B = 2;
	int32 C = 3;
	const TArray<int32*> Forward = {&A, &B, &C};
	const TArray<int32*> Shuffled = {&C, &A, &B};
	CHECK_EQUALS("Reordered pointer arrays should produce the same hash", FNArrayUtils::GetPointersHash(Forward), FNArrayUtils::GetPointersHash(Shuffled));
}

N_TEST_MEDIUM(FNArrayUtilsTests_GetPointersHash_DuplicateCancellation, "NEXUS::UnitTests::NCore::FNArrayUtils::GetPointersHash_DuplicateCancellation", N_TEST_CONTEXT_ANYWHERE)
{
	// Pins the documented XOR caveat: duplicate pointers cancel each other out, so a pair of the
	// same pointer hashes to zero and a duplicated pair leaves only the remaining element's hash.
	int32 A = 1;
	int32 B = 2;
	const TArray<int32*> DuplicatePair = {&A, &A};
	CHECK_EQUALS("A duplicated pointer pair should cancel to zero", FNArrayUtils::GetPointersHash(DuplicatePair), 0u);

	const TArray<int32*> PairWithRemainder = {&A, &B, &A};
	CHECK_EQUALS("A duplicated pair should leave only the remaining pointer's hash", FNArrayUtils::GetPointersHash(PairWithRemainder), GetTypeHash(&B));
}

#endif //WITH_TESTS
