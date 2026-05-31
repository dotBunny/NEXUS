// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * A collection of templated utility methods for working with TArrays.
 *
 * All helpers are inlined, allocation-free (or minimally allocating), and C++-only.
 */
class NEXUSCORE_API FNArrayUtils
{
public:
	/**
	 * Tests whether two arrays share at least one element.
	 * @param Left The first array to test.
	 * @param Right The second array to test.
	 * @return true if any element in Right is also present in Left, false otherwise.
	 */
	template<typename T>
	FORCEINLINE static bool ContainsAny(const TArray<T>& Left, const TArray<T>& Right)
	{
		return Right.FindByPredicate([&Left](const T& Name) {
			return Left.Contains(Name);
		}) != nullptr;
	}

	/**
	 * Compares two arrays for equality, element-by-element and in the same order.
	 * @param Left The first array to compare.
	 * @param Right The second array to compare.
	 * @return true if both arrays have the same length and identical elements at matching indices.
	 */
	template<typename T>
	FORCEINLINE static bool IsSameOrderedValues(const TArray<T>& Left, const TArray<T>& Right)
	{
		if (Left.Num() != Right.Num())
		{
			return false;
		}
		for (int32 i = 0; i < Left.Num(); i++)
		{
			if (Left[i] != Right[i]) return false;
		}
		return true;
	}

	/**
	 * Compares two arrays for equality without regard to ordering. Assumes elements are unique within each array.
	 * @param Left The first array to compare.
	 * @param Right The second array to compare.
	 * @return true if both arrays have the same length and contain the same set of elements.
	 */
	template<typename T>
	FORCEINLINE static bool IsSameUnorderedValues(const TArray<T>& Left, const TArray<T>& Right)
	{
		if (Left.Num() != Right.Num())
		{
			return false;
		}
		const TSet<T> LeftSet(Left);
		for (const T& Value : Right)
		{
			if (!LeftSet.Contains(Value)) return false;
		}
		return true;
	}

	/**
	 * Computes an order-independent hash derived from an array of pointers.
	 * @param Elements The pointers whose hashes should be combined.
	 * @return A 32-bit hash produced by XOR-combining each pointer's GetTypeHash value.
	 * @note Because the combination is a bitwise XOR, duplicate pointers will cancel each other out.
	 */
	template<typename T>
	FORCEINLINE static uint32 GetPointersHash(TArray<T*> Elements)
	{
		uint32 Hash = 0;
		for (T* Element : Elements)
		{
			Hash ^= GetTypeHash(Element);
		}
		return Hash;
	}

	/**
	 * Pins an array of weak object pointers, producing strong pointers that keep the objects alive.
	 * @param Objects The weak pointers to pin.
	 * @return A matching array of TStrongObjectPtr values; entries corresponding to stale weak pointers will be null.
	 */
	FORCEINLINE static TArray<TStrongObjectPtr<UObject>> PinAll(TArray<TWeakObjectPtr<UObject>> Objects)
	{
		TArray<TStrongObjectPtr<UObject>> Pinned;
		Pinned.Reserve(Objects.Num());
		for (const TWeakObjectPtr<UObject>& Weak : Objects)
		{
			Pinned.Emplace(Weak.Get()); // null for stale weak pointers
		}
		return Pinned;
	}
};