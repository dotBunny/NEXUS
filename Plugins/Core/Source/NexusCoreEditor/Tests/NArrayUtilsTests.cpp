// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArrayUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNArrayUtils_ContainsAny_EmptyArrays, "NEXUS::UnitTests::NCore::ArrayUtils::ContainsAny_EmptyArrays", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A;
	TArray<int32> B;
	CHECK_FALSE_MESSAGE(TEXT("Two empty arrays should not intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtils_ContainsAny_NoOverlap, "NEXUS::UnitTests::NCore::ArrayUtils::ContainsAny_NoOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {4, 5, 6};
	CHECK_FALSE_MESSAGE(TEXT("Non-overlapping arrays should not intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtils_ContainsAny_PartialOverlap, "NEXUS::UnitTests::NCore::ArrayUtils::ContainsAny_PartialOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {3, 4, 5};
	CHECK_MESSAGE(TEXT("Arrays sharing one element should intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtils_ContainsAny_FullOverlap, "NEXUS::UnitTests::NCore::ArrayUtils::ContainsAny_FullOverlap", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2, 3};
	CHECK_MESSAGE(TEXT("Identical arrays should intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtils_ContainsAny_SingleMatch, "NEXUS::UnitTests::NCore::ArrayUtils::ContainsAny_SingleMatch", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {10, 20, 30};
	TArray<int32> B = {30};
	CHECK_MESSAGE(TEXT("Single-element match should intersect"), FNArrayUtils::ContainsAny(A, B));
}

N_TEST_HIGH(FNArrayUtils_IsSameOrderedValues_Equal, "NEXUS::UnitTests::NCore::ArrayUtils::IsSameOrderedValues_Equal", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2, 3};
	CHECK_MESSAGE(TEXT("Identical arrays should match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtils_IsSameOrderedValues_DifferentSize, "NEXUS::UnitTests::NCore::ArrayUtils::IsSameOrderedValues_DifferentSize", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2};
	CHECK_FALSE_MESSAGE(TEXT("Arrays of different sizes should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtils_IsSameOrderedValues_DifferentOrder, "NEXUS::UnitTests::NCore::ArrayUtils::IsSameOrderedValues_DifferentOrder", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {3, 2, 1};
	CHECK_FALSE_MESSAGE(TEXT("Same values in different order should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtils_IsSameOrderedValues_BothEmpty, "NEXUS::UnitTests::NCore::ArrayUtils::IsSameOrderedValues_BothEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A;
	TArray<int32> B;
	CHECK_MESSAGE(TEXT("Two empty arrays should match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtils_IsSameOrderedValues_OneEmpty, "NEXUS::UnitTests::NCore::ArrayUtils::IsSameOrderedValues_OneEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B;
	CHECK_FALSE_MESSAGE(TEXT("Non-empty vs empty should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_MEDIUM(FNArrayUtils_IsSameOrderedValues_FNames, "NEXUS::UnitTests::NCore::ArrayUtils::IsSameOrderedValues_FNames", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<FName> A = {FName("Alpha"), FName("Beta"), FName("Gamma")};
	TArray<FName> B = {FName("Alpha"), FName("Beta"), FName("Gamma")};
	CHECK_MESSAGE(TEXT("Matching FName arrays should be equal"), FNArrayUtils::IsSameOrderedValues(A, B));

	B[2] = FName("Delta");
	CHECK_FALSE_MESSAGE(TEXT("FName arrays differing in last element should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

#endif //WITH_TESTS
