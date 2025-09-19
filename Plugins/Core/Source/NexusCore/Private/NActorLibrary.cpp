// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorLibrary.h"

bool UNActorLibrary::IsSameActors(const TArray<AActor*>& A, const TArray<AActor*>& B)
{
	const int32 Length = A.Num();
	if (Length != B.Num()) return false;
	
	TArray<bool> FindCache;
	FindCache.Reserve(Length);
	FindCache.AddDefaulted(Length);
	
	for (int32 i = 0; i < Length; i++)
	{
		bool Found = false;
		for (int32 j = 0; j < Length; j++)
		{
			if (FindCache[j]) continue;
			if (A[i] == B[j])
			{
				FindCache[j] = true;
				Found = true;
			}
		}
		if (!Found) return false;
	}
	return true;
}
