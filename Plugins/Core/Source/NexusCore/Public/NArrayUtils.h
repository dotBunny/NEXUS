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