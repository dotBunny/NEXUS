// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Collections/NWeightedIntegerArray.h"

void FNWeightedIntegerArray::Add(const int Value, const int Weight)
{
	if (Weight == 0) return;
	Data.Reserve(Data.Num() + Weight);
	for (int i = 0; i < Weight; i++)
	{
		Data.Add(Value);
	}
	CachedMaxIndex = Data.Num() - 1;
}

void FNWeightedIntegerArray::Empty()
{
	Data.Empty();
	CachedMaxIndex = -1;
}

void FNWeightedIntegerArray::Remove(const int Value)
{
	for (int i = Data.Num(); i >= 0; i--)
	{
		if (Data[i] == Value)
		{
			Data.RemoveAtSwap(i, EAllowShrinking::No);
		}
	}
	CachedMaxIndex = Data.Num() - 1;
}

void FNWeightedIntegerArray::RemoveSome(const int Value,  int Limit)
{
	for (int i = Data.Num(); i >= 0; i--)
	{
		if (Data[i] == Value)
		{
			Data.RemoveAtSwap(i, EAllowShrinking::No);
			Limit--;
		}

		if (Limit == 0) break;
	}
	CachedMaxIndex = Data.Num() - 1;
}
