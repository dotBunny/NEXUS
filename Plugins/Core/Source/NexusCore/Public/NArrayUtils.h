// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * A collection of utility methods for working with arrays.
 */
class NEXUSCORE_API FNArrayUtils
{
public:
	template<typename T>
	FORCEINLINE static bool ContainsAny(const TArray<T>& Left, const TArray<T>& Right)
	{
		return Right.FindByPredicate([&Left](const T& Name) {
			return Left.Contains(Name);
		}) != nullptr;
	}

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

	FORCEINLINE static TArray<TStrongObjectPtr<UObject>> PinAll(TArray<TWeakObjectPtr<UObject>> Objects)
	{
		const int32 Count = Objects.Num();
		TArray<TStrongObjectPtr<UObject>> Pinned;
		Pinned.SetNumUninitialized(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			Pinned[i] = Objects[i].Pin();
		}
		return MoveTemp(Pinned);
	}

	template<typename T>
	FORCEINLINE static TArray<T> PinAllAs(TArray<TWeakObjectPtr<UObject>> Objects)
	{
		const int32 Count  = Objects.Num();
		TArray<T> PinnedObjects;
		PinnedObjects.SetNumUninitialized(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			PinnedObjects[i] = static_cast<T>(Objects[i].Pin().Get());
		}
		return MoveTemp(PinnedObjects);
	}
};