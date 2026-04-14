// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArrayUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNArrayUtilsTests_ContainsAny_EmptyArrays, "NEXUS::UnitTests::NCore::FNArrayUtils::ContainsAny_EmptyArrays", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A;
	TArray<int32> B;
	CHECK_FALSE_MESSAGE(TEXT("Two empty arrays should not intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_ContainsAny_NoOverlap, "NEXUS::UnitTests::NCore::FNArrayUtils::ContainsAny_NoOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {4, 5, 6};
	CHECK_FALSE_MESSAGE(TEXT("Non-overlapping arrays should not intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_ContainsAny_PartialOverlap, "NEXUS::UnitTests::NCore::FNArrayUtils::ContainsAny_PartialOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {3, 4, 5};
	CHECK_MESSAGE(TEXT("Arrays sharing one element should intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_ContainsAny_FullOverlap, "NEXUS::UnitTests::NCore::FNArrayUtils::ContainsAny_FullOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2, 3};
	CHECK_MESSAGE(TEXT("Identical arrays should intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_ContainsAny_SingleMatch, "NEXUS::UnitTests::NCore::FNArrayUtils::ContainsAny_SingleMatch", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {10, 20, 30};
	TArray<int32> B = {30};
	CHECK_MESSAGE(TEXT("Single-element match should intersect"), FNArrayUtils::ContainsAny(A, B));
}

#endif //WITH_TESTS
