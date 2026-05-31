// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NArrayUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNArrayUtilsTests_IsSameOrderedValues_Equal, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameOrderedValues_Equal", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2, 3};
	CHECK_MESSAGE(TEXT("Identical arrays should match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameOrderedValues_DifferentSize, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameOrderedValues_DifferentSize", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {1, 2};
	CHECK_FALSE_MESSAGE(TEXT("Arrays of different sizes should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameOrderedValues_DifferentOrder, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameOrderedValues_DifferentOrder", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B = {3, 2, 1};
	CHECK_FALSE_MESSAGE(TEXT("Same values in different order should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameOrderedValues_BothEmpty, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameOrderedValues_BothEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A;
	TArray<int32> B;
	CHECK_MESSAGE(TEXT("Two empty arrays should match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_HIGH(FNArrayUtilsTests_IsSameOrderedValues_OneEmpty, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameOrderedValues_OneEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<int32> A = {1, 2, 3};
	TArray<int32> B;
	CHECK_FALSE_MESSAGE(TEXT("Non-empty vs empty should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

N_TEST_MEDIUM(FNArrayUtilsTests_IsSameOrderedValues_FNames, "NEXUS::UnitTests::NCore::FNArrayUtils::IsSameOrderedValues_FNames", N_TEST_CONTEXT_ANYWHERE)
{
	TArray<FName> A = {FName("Alpha"), FName("Beta"), FName("Gamma")};
	TArray<FName> B = {FName("Alpha"), FName("Beta"), FName("Gamma")};
	CHECK_MESSAGE(TEXT("Matching FName arrays should be equal"), FNArrayUtils::IsSameOrderedValues(A, B));

	B[2] = FName("Delta");
	CHECK_FALSE_MESSAGE(TEXT("FName arrays differing in last element should not match"), FNArrayUtils::IsSameOrderedValues(A, B));
}

#endif //WITH_TESTS
