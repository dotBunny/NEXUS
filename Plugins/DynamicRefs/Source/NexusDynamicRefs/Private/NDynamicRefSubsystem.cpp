// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefSubsystem.h"

#include "GameplayTagsManager.h"
#include "NDynamicRefsMinimal.h"
#include "Macros/NValidationMacros.h"

void UNDynamicRefSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Pre-allocate space for enum based collection
	FastCollection.AddZeroed(NDR_Max);

	Super::Initialize(Collection);
}

void UNDynamicRefSubsystem::Deinitialize()
{
	FastCollection.Empty();
	NamedCollection.Empty();

	Super::Deinitialize();
}

void UNDynamicRefSubsystem::AddObject(const ENDynamicRef DynamicRef, UObject* InObject)
{
	N_VALIDATE_RETURN_VOID(LogNexusDynamicRefs, InObject);

	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to add UObject(%s) to NDR_None/NDR_Max, skipping."), *InObject->GetName());
		return;
	}

	FNDynamicRefCollection& Collection = FastCollection[DynamicRef];

	// Prune stale entries on the write path so the read accessors can stay const. Add already scans the whole
	// array (AddUnique), so folding the compaction in here keeps the call the same O(n) order.
	Collection.Compact();

	// Only broadcast when the object was genuinely added; a duplicate AddUnique is a no-op and must not
	// re-fire OnAdded, otherwise the add/remove delegate pair becomes asymmetric for external listeners.
	if (Collection.Add(InObject))
	{
		OnAdded.Broadcast(DynamicRef, InObject);
	}
}

void UNDynamicRefSubsystem::AddObjects(const ENDynamicRef DynamicRef, const TArray<UObject*>& InObjects)
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to add UObjects to NDR_None/NDR_Max, skipping."));
		return;
	}

	// Compact once for the whole batch rather than per element (routing through AddObject would prune on every call).
	FNDynamicRefCollection& Collection = FastCollection[DynamicRef];
	Collection.Compact();

	for (UObject* Object : InObjects)
	{
		N_VALIDATE_CONTINUE(LogNexusDynamicRefs, Object);
		if (Collection.Add(Object))
		{
			OnAdded.Broadcast(DynamicRef, Object);
		}
	}
}

void UNDynamicRefSubsystem::AddObjectByName(const FName Name, UObject* InObject)
{
	N_VALIDATE_RETURN_VOID(LogNexusDynamicRefs, InObject);

	FNDynamicRefCollection& Collection = NamedCollection.FindOrAdd(Name);
	Collection.Compact();
	if (Collection.Add(InObject))
	{
		OnAddedByName.Broadcast(Name, InObject);
	}
}

void UNDynamicRefSubsystem::AddObjectsByName(const FName Name, const TArray<UObject*>& InObjects)
{
	// Validate objects to ensure we are not creating an empty bucket
	bool bHasObjects = false;
	for (const UObject* Object : InObjects)
	{
		if (IsValid(Object))
		{
			bHasObjects = true;
			break;
		}
	}
	if (!bHasObjects) return;

	// Go about adding
	FNDynamicRefCollection& Collection = NamedCollection.FindOrAdd(Name);
	Collection.Compact();
	for (UObject* Object : InObjects)
	{
		N_VALIDATE_CONTINUE(LogNexusDynamicRefs, Object);
		if (Collection.Add(Object))
		{
			OnAddedByName.Broadcast(Name, Object);
		}
	}
}

void UNDynamicRefSubsystem::RemoveObject(const ENDynamicRef DynamicRef, UObject* InObject)
{
	if (InObject == nullptr)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to remove a null UObject, skipping."));
		return;
	}

	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to remove UObject(%s) from NDR_None/NDR_Max, skipping."), *InObject->GetName());
		return;
	}
	// Only remove and callback if we have the actual objects
	if (FastCollection[DynamicRef].Remove(InObject))
	{
		OnRemoved.Broadcast(DynamicRef, InObject);
	}
}

void UNDynamicRefSubsystem::RemoveObjects(const ENDynamicRef DynamicRef, const TArray<UObject*>& InObjects)
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to remove UObjects from NDR_None/NDR_Max, skipping."));
		return;
	}

	for (UObject* Object : InObjects)
	{
		RemoveObject(DynamicRef, Object);
	}
}

void UNDynamicRefSubsystem::RemoveObjectByName(const FName Name, UObject* InObject)
{
	FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr && Collection->Remove(InObject))
	{
		OnRemovedByName.Broadcast(Name, InObject);

		// Remove empty collection
		if (!Collection->HasObjects())
		{
			NamedCollection.Remove(Name);
		}
	}

}

void UNDynamicRefSubsystem::RemoveObjectsByName(const FName Name, const TArray<UObject*>& InObjects)
{
	FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		for (UObject* Object : InObjects)
		{
			if (Collection->Remove(Object))
			{
				OnRemovedByName.Broadcast(Name, Object);
			}
		}

		if (!Collection->HasObjects())
		{
			NamedCollection.Remove(Name);
		}
	}
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjects(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get objects from NDR_None/NDR_Max, returning empty."));
		return TArray<UObject*>();
	}

	return FastCollection[DynamicRef].GetObjectsCopy();
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByName(const FName Name) const
{
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		return Collection->GetObjectsCopy();
	}
	return TArray<UObject*>();
}

AActor* UNDynamicRefSubsystem::GetFirstActor(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get first actor from NDR_None/NDR_Max, returning null."));
		return nullptr;
	}

	for (const TWeakObjectPtr<UObject>& ObjPtr : FastCollection[DynamicRef].Objects)
	{
		AActor* Actor = Cast<AActor>(ObjPtr.Get());
		if (Actor != nullptr)
		{
			return Actor;
		}
	}
	return nullptr;
}

AActor* UNDynamicRefSubsystem::GetFirstActorByName(const FName Name) const
{
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection == nullptr) return nullptr;
	for (const TWeakObjectPtr<UObject>& ObjPtr : Collection->Objects)
	{
		AActor* Actor = Cast<AActor>(ObjPtr.Get());
		if (Actor != nullptr) return Actor;
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetFirstObject(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get first object from NDR_None/NDR_Max, returning null."));
		return nullptr;
	}

	return FastCollection[DynamicRef].GetFirstValid();
}

UObject* UNDynamicRefSubsystem::GetFirstObjectUnsafe(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max) return nullptr;
	return FastCollection[DynamicRef].Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetLastObject(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get last object from NDR_None/NDR_Max, returning null."));
		return nullptr;
	}

	return FastCollection[DynamicRef].GetLastValid();
}

UObject* UNDynamicRefSubsystem::GetLastObjectUnsafe(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max) return nullptr;
	return FastCollection[DynamicRef].Objects.Last().Get();
}

TArray<AActor*> UNDynamicRefSubsystem::GetActors(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested AActors from NDR_None/NDR_Max, returning empty."));
		return TArray<AActor*>();
	}

	TArray<AActor*> Result;
	for (UObject* Object : FastCollection[DynamicRef].GetObjectsCopy())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return Result;
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByName(const FName Name) const
{
	TArray<AActor*> Result;
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection == nullptr) return Result;
	for (const TWeakObjectPtr<UObject>& ObjPtr : Collection->Objects)
	{
		AActor* Actor = Cast<AActor>(ObjPtr.Get());
		if (Actor) Result.Add(Actor);
	}
	return Result;
}

int32 UNDynamicRefSubsystem::GetCount(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested count of NDR_None/NDR_Max, returning 0."));
		return 0;
	}
	// Counts live entries without pruning; stale entries are cleaned up on the next add to this slot.
	return FastCollection[DynamicRef].CountValid();
}

int32 UNDynamicRefSubsystem::GetCountByName(const FName Name) const
{
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		return Collection->CountValid();
	}
	return 0;
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByName(const FName Name) const
{
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		return Collection->GetFirstValid();
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByNameUnsafe(const FName Name) const
{
	return NamedCollection[Name].Objects[0].Get();
}

AActor* UNDynamicRefSubsystem::GetLastActor(const ENDynamicRef DynamicRef) const
{
	if (DynamicRef == NDR_None || DynamicRef == NDR_Max)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested last AActor from NDR_None/NDR_Max, returning null."));
		return nullptr;
	}

	const TArray<TWeakObjectPtr<UObject>>& Objects = FastCollection[DynamicRef].Objects;
	for (int32 i = Objects.Num() - 1; i >= 0; --i)
	{
		AActor* Actor = Cast<AActor>(Objects[i].Get());
		if (Actor) return Actor;
	}
	return nullptr;
}

AActor* UNDynamicRefSubsystem::GetLastActorByName(const FName Name) const
{
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection == nullptr) return nullptr;
	const TArray<TWeakObjectPtr<UObject>>& Objects = Collection->Objects;
	for (int32 i = Objects.Num() - 1; i >= 0; --i)
	{
		AActor* Actor = Cast<AActor>(Objects[i].Get());
		if (Actor) return Actor;
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetLastObjectByName(const FName Name) const
{
	const FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		return Collection->GetLastValid();
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetLastObjectByNameUnsafe(const FName Name) const
{
	return NamedCollection[Name].Objects.Last().Get();
}

TArray<FName> UNDynamicRefSubsystem::GetNames() const
{
	TArray<FName> Result;
	Result.Reserve(NamedCollection.Num());
	for (const TPair<FName, FNDynamicRefCollection>& Pair : NamedCollection)
	{
		// Skip buckets whose objects have all gone stale so callers only see live names.
		if (Pair.Value.HasObjects())
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

TArray<ENDynamicRef> UNDynamicRefSubsystem::GetDynamicRefs() const
{
	TArray<ENDynamicRef> Result;
	for (int32 i = 0; i < NDR_Max; i++)
	{
		if (FastCollection[i].HasObjects())
		{
			Result.Add(static_cast<ENDynamicRef>(i));
		}
	}
	return Result;
}

const FNDynamicRefCollection& UNDynamicRefSubsystem::GetObjectCollectionRefUnsafe(const ENDynamicRef DynamicRef) const
{
	return FastCollection[DynamicRef];
}

const FNDynamicRefCollection& UNDynamicRefSubsystem::GetObjectCollectionByNameRefUnsafe(const FName Name) const
{
	return NamedCollection.FindChecked(Name);
}

TArray<FGameplayTag> UNDynamicRefSubsystem::GetTags() const
{
	TArray<FGameplayTag> Result;
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	for (const TPair<FName, FNDynamicRefCollection>& Pair : NamedCollection)
	{
		// Skip buckets whose objects have all gone stale so callers only see live tags.
		if (!Pair.Value.HasObjects()) continue;
		FGameplayTag Tag = Manager.RequestGameplayTag(Pair.Key, false);
		if (Tag.IsValid())
		{
			Result.Add(Tag);
		}
	}
	return Result;
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return TArray<AActor*>();
	return GetActorsByName(Tag.GetTagName());
}

int32 UNDynamicRefSubsystem::GetCountByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return 0;
	return GetCountByName(Tag.GetTagName());
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return TArray<UObject*>();
	return GetObjectsByName(Tag.GetTagName());
}

AActor* UNDynamicRefSubsystem::GetFirstActorByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return nullptr;
	return GetFirstActorByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return nullptr;
	return GetFirstObjectByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByTagUnsafe(FGameplayTag Tag) const
{
	return GetFirstObjectByNameUnsafe(Tag.GetTagName());
}

AActor* UNDynamicRefSubsystem::GetLastActorByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return nullptr;
	return GetLastActorByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetLastObjectByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return nullptr;
	return GetLastObjectByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetLastObjectByTagUnsafe(FGameplayTag Tag) const
{
	return GetLastObjectByNameUnsafe(Tag.GetTagName());
}

const FNDynamicRefCollection& UNDynamicRefSubsystem::GetObjectCollectionByTagRefUnsafe(FGameplayTag Tag) const
{
	return NamedCollection.FindChecked(Tag.GetTagName());
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByAnyTags(const FGameplayTagContainer& Tags) const
{
	TArray<UObject*> Result;
	if (Tags.IsEmpty()) return Result;

	TArray<FGameplayTag> TagArray;
	Tags.GetGameplayTagArray(TagArray);

	int32 ReserveCount = 0;
	for (const FGameplayTag& Tag : TagArray)
	{
		if (!Tag.IsValid()) continue;
		if (const FNDynamicRefCollection* Collection = NamedCollection.Find(Tag.GetTagName()))
		{
			ReserveCount += Collection->Objects.Num();
		}
	}
	Result.Reserve(ReserveCount);

	// Track seen objects in a set for O(1) dedup while preserving first-seen order in Result.
	TSet<UObject*> Seen;
	Seen.Reserve(ReserveCount);
	for (const FGameplayTag& Tag : TagArray)
	{
		if (!Tag.IsValid()) continue;
		const FNDynamicRefCollection* Collection = NamedCollection.Find(Tag.GetTagName());
		if (Collection == nullptr) continue;
		for (const TWeakObjectPtr<UObject>& ObjPtr : Collection->Objects)
		{
			if (UObject* Object = ObjPtr.Get())
			{
				bool bAlreadySeen = false;
				Seen.Add(Object, &bAlreadySeen);
				if (!bAlreadySeen)
				{
					Result.Add(Object);
				}
			}
		}
	}
	return Result;
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByAnyTags(const FGameplayTagContainer& Tags) const
{
	TArray<AActor*> Result;
	for (UObject* Object : GetObjectsByAnyTags(Tags))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return Result;
}

int32 UNDynamicRefSubsystem::GetCountByAnyTags(const FGameplayTagContainer& Tags) const
{
	return GetObjectsByAnyTags(Tags).Num();
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByAllTags(const FGameplayTagContainer& Tags) const
{
	TArray<UObject*> Result;
	if (Tags.IsEmpty()) return Result;

	TArray<FGameplayTag> TagArray;
	Tags.GetGameplayTagArray(TagArray);

	// Resolve every requested tag to its bucket; if any tag is missing, the intersection is empty.
	TArray<const FNDynamicRefCollection*> Collections;
	Collections.Reserve(TagArray.Num());
	for (const FGameplayTag& Tag : TagArray)
	{
		if (!Tag.IsValid()) return Result;
		const FNDynamicRefCollection* Collection = NamedCollection.Find(Tag.GetTagName());
		if (Collection == nullptr || !Collection->HasObjects()) return Result;
		Collections.Add(Collection);
	}

	// Pick the smallest bucket as the iteration source to minimize work.
	int32 SmallestIndex = 0;
	for (int32 i = 1; i < Collections.Num(); ++i)
	{
		if (Collections[i]->Objects.Num() < Collections[SmallestIndex]->Objects.Num())
		{
			SmallestIndex = i;
		}
	}

	const FNDynamicRefCollection* Smallest = Collections[SmallestIndex];
	Result.Reserve(Smallest->Objects.Num());
	for (const TWeakObjectPtr<UObject>& ObjPtr : Smallest->Objects)
	{
		UObject* Object = ObjPtr.Get();
		if (Object == nullptr) continue;
		const TWeakObjectPtr<UObject> WeakObject(Object);
		bool bInAll = true;
		for (int32 i = 0; i < Collections.Num(); ++i)
		{
			if (i == SmallestIndex) continue;
			if (!Collections[i]->Objects.Contains(WeakObject))
			{
				bInAll = false;
				break;
			}
		}
		if (bInAll) Result.Add(Object);
	}
	return Result;
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByAllTags(const FGameplayTagContainer& Tags) const
{
	TArray<AActor*> Result;
	for (UObject* Object : GetObjectsByAllTags(Tags))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return Result;
}

int32 UNDynamicRefSubsystem::GetCountByAllTags(const FGameplayTagContainer& Tags) const
{
	return GetObjectsByAllTags(Tags).Num();
}