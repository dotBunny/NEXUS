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
	CHECK_FALSE_MESSAGE(TEXT("Value 7 should no longer be present after Remove(7)"), Array.HasValue(7));
	CHECK_MESSAGE(TEXT("Value 8 should still be present after Remove(7)"), Array.HasValue(8));
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_Remove_LeavesOtherValuesIntact, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::Remove_LeavesOtherValuesIntact", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that Remove only deletes copies of the requested value and that the remaining
	// unique-value set is exactly { 2, 4 } when 1 and 3 are removed from a four-value array.
	FNWeightedIntegerArray Array;
	Array.Add(1, 2);
	Array.Add(2, 3);
	Array.Add(3, 1);
	Array.Add(4, 4);
	Array.Remove(1);
	Array.Remove(3);
	CHECK_EQUALS("WeightedCount should be the sum of remaining weights (3 + 4)", Array.WeightedCount(), 7);

	const TArray<int32> Unique = Array.GetUniqueValues();
	CHECK_EQUALS("Exactly two distinct values should remain", Unique.Num(), 2);
	CHECK_MESSAGE(TEXT("Value 2 should be retained"), Unique.Contains(2));
	CHECK_MESSAGE(TEXT("Value 4 should be retained"), Unique.Contains(4));
	CHECK_FALSE_MESSAGE(TEXT("Value 1 should be absent after Remove"), Array.HasValue(1));
	CHECK_FALSE_MESSAGE(TEXT("Value 3 should be absent after Remove"), Array.HasValue(3));
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_RemoveSome_PartialRemoval, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::RemoveSome_PartialRemoval", N_TEST_CONTEXT_ANYWHERE)
{
	FNWeightedIntegerArray Array;
	Array.Add(5, 10);
	Array.RemoveSome(5, 3);
	CHECK_EQUALS("Removing 3 of 10 entries should leave 7", Array.WeightedCount(), 7);
	CHECK_MESSAGE(TEXT("Value 5 should still be present after partial removal"), Array.HasValue(5));
}

N_TEST_HIGH(FNWeightedIntegerArrayTests_RemoveSome_AcrossMultipleValues, "NEXUS::UnitTests::NCore::FNWeightedIntegerArray::RemoveSome_AcrossMultipleValues", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that RemoveSome only removes copies of the requested value and never touches other
	// values when both are present in the array.
	FNWeightedIntegerArray Array;
	Array.Add(10, 4);
	Array.Add(20, 4);
	Array.RemoveSome(10, 2);
	CHECK_EQUALS("Total count should drop by exactly 2", Array.WeightedCount(), 6);
	CHECK_MESSAGE(TEXT("Value 10 should still be present (only 2 of 4 removed)"), Array.HasValue(10));
	CHECK_MESSAGE(TEXT("Value 20 should be untouched"), Array.HasValue(20));

	const TArray<int32> Unique = Array.GetUniqueValues();
	CHECK_EQUALS("Exactly two distinct values should remain", Unique.Num(), 2);
	CHECK_MESSAGE(TEXT("Unique set should still contain 10"), Unique.Contains(10));
	CHECK_MESSAGE(TEXT("Unique set should still contain 20"), Unique.Contains(20));
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
	CHECK_FALSE_MESSAGE(TEXT("Value 99 should be fully removed"), Array.HasValue(99));
	CHECK_MESSAGE(TEXT("Value 100 should be the surviving entry"), Array.HasValue(100));
}

#endif //WITH_TESTS
