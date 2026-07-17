// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArrayUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNArrayUtilsTests_IsSameUnorderedValues_EmptyArrays, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameUnorderedValues_EmptyArrays", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A;
	TArray<int32> B;
	CHECK_MESSAGE(TEXT("Two empty arrays should be considered the same set"), FNArrayUtils::IsSameUnorderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameUnorderedValues_SameOrder, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameUnorderedValues_SameOrder", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2, 3};
	CHECK_MESSAGE(TEXT("Identical arrays should be the same set"), FNArrayUtils::IsSameUnorderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameUnorderedValues_DifferentOrder, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameUnorderedValues_DifferentOrder", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the core contract: ordering must not affect set equality.
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {3, 1, 2};
	CHECK_MESSAGE(TEXT("Reordered arrays should be the same set"), FNArrayUtils::IsSameUnorderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameUnorderedValues_DifferentLengths, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameUnorderedValues_DifferentLengths", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2};
	CHECK_FALSE_MESSAGE(TEXT("Arrays of different lengths should not be the same set"), FNArrayUtils::IsSameUnorderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameUnorderedValues_DisjointValues, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameUnorderedValues_DisjointValues", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {4, 5, 6};
	CHECK_FALSE_MESSAGE(TEXT("Disjoint arrays should not be the same set"), FNArrayUtils::IsSameUnorderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameUnorderedValues_PartialOverlap, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameUnorderedValues_PartialOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2, 4};
	CHECK_FALSE_MESSAGE(TEXT("Equal-length arrays differing by one element should not be the same set"), FNArrayUtils::IsSameUnorderedValues(A, B));
}

#endif //WITH_TESTS
