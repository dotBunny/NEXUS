// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorLibrary.h"

bool UNActorLibrary::IsSameActors(const TArray<AActor*>& A, const TArray<AActor*>& B)
{
	const int32 Length = A.Num();
	if (Length != B.Num()) return false;

	TMap<AActor*, int32> Counts;
	Counts.Reserve(Length);
	for (AActor* Actor : A)
	{
		++Counts.FindOrAdd(Actor);
	}

	for (AActor* Actor : B)
	{
		int32* Count = Counts.Find(Actor);
		if (Count == nullptr || *Count == 0) return false;
		--(*Count);
	}

	return true;
}

TArray<AActor*> UNActorLibrary::ToActorArray(const TArray<UObject*> InObjects)
{
	TArray<AActor*> ReturnArray;
	for (UObject* Object : InObjects)
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor != nullptr) ReturnArray.Add(Actor);
	}
	return ReturnArray;
}
