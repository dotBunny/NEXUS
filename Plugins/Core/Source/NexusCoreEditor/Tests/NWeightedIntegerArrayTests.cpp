// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Collections/NWeightedIntegerArray.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNWeightedIntegerArrayTests_HasData_EmptyArray, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::HasData_EmptyArray", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	CHECK_FALSE_MESSAGE(TEXT("Newly created array should have no data"), Array.HasData());
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_Add_SingleValue, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::Add_SingleValue", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(42);
	CHECK_MESSAGE(TEXT("Array should have data after adding"), Array.HasData());
	CHECK_EQUALS("Count should be 1 after adding one value with default weight", Array.WeightedCount(), 1);
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_Add_WithWeight, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::Add_WithWeight", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(10, 5);
	CHECK_EQUALS("Count should equal the weight when adding a single value", Array.WeightedCount(), 5);
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_Remove_AllInstances, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::Remove_AllInstances", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(7, 3);
	Array.Add(8, 2);
	Array.Remove(7);
	CHECK_EQUALS("Only value 8 entries should remain after removing 7", Array.WeightedCount(), 2);
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_RemoveSome_PartialRemoval, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::RemoveSome_PartialRemoval", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(5, 10);
	Array.RemoveSome(5, 3);
	CHECK_EQUALS("Removing 3 of 10 entries should leave 7", Array.WeightedCount(), 7);
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_Empty_ClearsAll, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::Empty_ClearsAll", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(1, 5);
	Array.Add(2, 5);
	Array.Empty();
	CHECK_FALSE_MESSAGE(TEXT("Array should have no data after Empty()"), Array.HasData());
	CHECK_EQUALS("Count should be 0 after Empty()", Array.WeightedCount(), 0);
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_Add_MultipleValues, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::Add_MultipleValues", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(1, 3);
	Array.Add(2, 7);
	CHECK_EQUALS("Total count should be sum of all weights", Array.WeightedCount(), 10);
}

N_TEST_MEDIUM(FNWeightedIntegerArrayTests_RemoveSome_MoreThanExist, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::RemoveSome_MoreThanExist", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(99, 2);
	Array.Add(100, 3);
	Array.RemoveSome(99, 100);
	CHECK_EQUALS("Removing more than exist should remove all instances of that value", Array.WeightedCount(), 3);
}

#endif //WITH_TESTS
