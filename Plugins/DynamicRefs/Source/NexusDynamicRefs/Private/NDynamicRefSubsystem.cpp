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
	if (InObject == nullptr)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to add a null UObject, skipping."));
		return;
	}
	
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to add UObject(%s) to NDR_None, skipping."), *InObject->GetName());
		return;
	}
	
	N_VALIDATE_RETURN_VOID(LogNexusDynamicRefs, InObject)
	FastCollection[DynamicRef].Add(InObject);
	OnAdded.Broadcast(DynamicRef, InObject);
}

void UNDynamicRefSubsystem::AddObjects(const ENDynamicRef DynamicRef, TArray<UObject*> InObjects)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to add UObjects to NDR_None, skipping."));
		return;
	}
	
	for (UObject* Object : InObjects)
	{
		N_VALIDATE_CONTINUE(LogNexusDynamicRefs, Object)
		AddObject(DynamicRef, Object);
	}
}

void UNDynamicRefSubsystem::AddObjectByName(const FName Name, UObject* InObject)
{
	N_VALIDATE_RETURN_VOID(LogNexusDynamicRefs, InObject)
	
	FNDynamicRefCollection& Collection = NamedCollection.FindOrAdd(Name);
	Collection.Add(InObject);
	OnAddedByName.Broadcast(Name, InObject);
}

void UNDynamicRefSubsystem::AddObjectsByName(const FName Name, TArray<UObject*> InObjects)
{
	FNDynamicRefCollection& Collection = NamedCollection.FindOrAdd(Name);
	for (UObject* Object : InObjects)
	{
		N_VALIDATE_CONTINUE(LogNexusDynamicRefs, Object)
		Collection.Add(Object);
		OnAddedByName.Broadcast(Name, Object);
	}
}

void UNDynamicRefSubsystem::RemoveObject(const ENDynamicRef DynamicRef, UObject* InObject)
{
	if (InObject == nullptr)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to remove a null UObject, skipping."));
		return;
	}
	
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to remove UObject(%s) from NDR_None, skipping."), *InObject->GetName());
		return;
	}
	// Only remove and callback if we have the actual objects
	if (FastCollection[DynamicRef].Remove(InObject))
	{
		OnRemoved.Broadcast(DynamicRef, InObject);
	}
}

void UNDynamicRefSubsystem::RemoveObjects(const ENDynamicRef DynamicRef, TArray<UObject*> InObjects)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to remove UObjects from NDR_None, skipping."));
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

void UNDynamicRefSubsystem::RemoveObjectsByName(const FName Name, TArray<UObject*> InObjects)
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

TArray<UObject*> UNDynamicRefSubsystem::GetObjects(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get objects from NDR_None, returning empty."));
		return TArray<UObject*>();;
	}
	
	return FastCollection[DynamicRef].GetObjectsCopy();
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByName(const FName Name)
{
	FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		return Collection->GetObjectsCopy();
	}
	return TArray<UObject*>();
}

AActor* UNDynamicRefSubsystem::GetFirstActor(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get first actor from NDR_None, returning null."));
		return nullptr;
	}
	
	for (const TObjectPtr<UObject>& ObjPtr : FastCollection[DynamicRef].Objects)
	{
		AActor* Actor = Cast<AActor>(ObjPtr.Get());
		if (Actor != nullptr)
		{
			return Actor;
		}
	}
	return nullptr;
}

AActor* UNDynamicRefSubsystem::GetFirstActorByName(const FName Name)
{
	for (UObject* Object : GetObjectsByName(Name))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) return Actor;
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetFirstObject(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get first object from NDR_None, returning null."));
		return nullptr;
	}

	FNDynamicRefCollection& Collection = FastCollection[DynamicRef];
	if (!Collection.HasObjects()) return nullptr;
	return Collection.Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetFirstObjectUnsafe(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None) return nullptr;
	return FastCollection[DynamicRef].Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetLastObject(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Attempted to get last object from NDR_None, returning null."));
		return nullptr;
	}
	
	FNDynamicRefCollection& Collection = FastCollection[DynamicRef];
	if (!Collection.HasObjects()) return nullptr;
	return Collection.Objects.Last();
}

UObject* UNDynamicRefSubsystem::GetLastObjectUnsafe(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None) return nullptr;
	return FastCollection[DynamicRef].Objects.Last();
}

TArray<AActor*> UNDynamicRefSubsystem::GetActors(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested AActors from NDR_None, returning empty."));
		return TArray<AActor*>();
	}
	
	TArray<AActor*> Result;
	for (UObject* Object : FastCollection[DynamicRef].GetObjectsCopy())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return MoveTemp(Result);
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByName(const FName Name)
{
	TArray<AActor*> Result;
	for (UObject* Object : GetObjectsByName(Name))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return MoveTemp(Result);
}

int32 UNDynamicRefSubsystem::GetCount(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested count of NDR_None, returning 0."));
		return 0;
	}
	return FastCollection[DynamicRef].Objects.Num();
}

int32 UNDynamicRefSubsystem::GetCountByName(const FName Name)
{
	FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr)
	{
		return Collection->Objects.Num();
	}
	return 0;
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByName(const FName Name)
{
	FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr && Collection->HasObjects())
	{
		return Collection->Objects[0];
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByNameUnsafe(const FName Name)
{
	return NamedCollection[Name].Objects[0];
}

AActor* UNDynamicRefSubsystem::GetLastActor(const ENDynamicRef DynamicRef)
{
	if (DynamicRef == NDR_None)
	{
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested last AActor from NDR_None, returning null."));
		return nullptr;
	}
	
	TArray<UObject*> Objects = GetObjects(DynamicRef);
	const int32 ObjectsCount = Objects.Num();
	for (int32 i = ObjectsCount - 1; i >= 0; i--)
	{
		AActor* Actor = Cast<AActor>(Objects[i]);
		if (Actor) return Actor;
	}
	return nullptr;
}

AActor* UNDynamicRefSubsystem::GetLastActorByName(const FName Name)
{
	TArray<UObject*> Objects = GetObjectsByName(Name);
	const int32 ObjectsCount = Objects.Num();
	for (int32 i = ObjectsCount - 1; i >= 0; i--)
	{
		AActor* Actor = Cast<AActor>(Objects[i]);
		if (Actor) return Actor;
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetLastObjectByName(const FName Name)
{
	FNDynamicRefCollection* Collection = NamedCollection.Find(Name);
	if (Collection != nullptr && Collection->HasObjects())
	{
		return Collection->Objects.Last().Get();
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetLastObjectByNameUnsafe(const FName Name)
{
	return NamedCollection[Name].Objects.Last();
}

TArray<FName> UNDynamicRefSubsystem::GetNames() const
{
	TArray<FName> Result;
	NamedCollection.GetKeys(Result);
	return MoveTemp(Result);
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
	return MoveTemp(Result);
}

const FNDynamicRefCollection& UNDynamicRefSubsystem::GetObjectCollectionRefUnsafe(const ENDynamicRef DynamicRef)
{
	return FastCollection[DynamicRef];
}

const FNDynamicRefCollection& UNDynamicRefSubsystem::GetObjectCollectionByNameRefUnsafe(const FName Name)
{
	return NamedCollection.FindChecked(Name);
}

TArray<FGameplayTag> UNDynamicRefSubsystem::GetTags() const
{
	TArray<FGameplayTag> Result;
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	for (const TPair<FName, FNDynamicRefCollection>& Pair : NamedCollection)
	{
		FGameplayTag Tag = Manager.RequestGameplayTag(Pair.Key, false);
		if (Tag.IsValid())
		{
			Result.Add(Tag);
		}
	}
	return MoveTemp(Result);
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return TArray<AActor*>();
	return GetActorsByName(Tag.GetTagName());
}

int32 UNDynamicRefSubsystem::GetCountByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return 0;
	return GetCountByName(Tag.GetTagName());
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return TArray<UObject*>();
	return GetObjectsByName(Tag.GetTagName());
}

AActor* UNDynamicRefSubsystem::GetFirstActorByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return nullptr;
	return GetFirstActorByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return nullptr;
	return GetFirstObjectByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByTagUnsafe(FGameplayTag Tag)
{
	return GetFirstObjectByNameUnsafe(Tag.GetTagName());
}

AActor* UNDynamicRefSubsystem::GetLastActorByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return nullptr;
	return GetLastActorByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetLastObjectByTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return nullptr;
	return GetLastObjectByName(Tag.GetTagName());
}

UObject* UNDynamicRefSubsystem::GetLastObjectByTagUnsafe(FGameplayTag Tag)
{
	return GetLastObjectByNameUnsafe(Tag.GetTagName());
}

const FNDynamicRefCollection& UNDynamicRefSubsystem::GetObjectCollectionByTagRefUnsafe(FGameplayTag Tag)
{
	return NamedCollection.FindChecked(Tag.GetTagName());
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByAnyTags(const FGameplayTagContainer& Tags)
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

	for (const FGameplayTag& Tag : TagArray)
	{
		if (!Tag.IsValid()) continue;
		const FNDynamicRefCollection* Collection = NamedCollection.Find(Tag.GetTagName());
		if (Collection == nullptr) continue;
		for (const TObjectPtr<UObject>& ObjPtr : Collection->Objects)
		{
			Result.AddUnique(ObjPtr.Get());
		}
	}
	return MoveTemp(Result);
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByAnyTags(const FGameplayTagContainer& Tags)
{
	TArray<AActor*> Result;
	for (UObject* Object : GetObjectsByAnyTags(Tags))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return MoveTemp(Result);
}

int32 UNDynamicRefSubsystem::GetCountByAnyTags(const FGameplayTagContainer& Tags)
{
	return GetObjectsByAnyTags(Tags).Num();
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByAllTags(const FGameplayTagContainer& Tags)
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
	for (const TObjectPtr<UObject>& ObjPtr : Smallest->Objects)
	{
		UObject* Object = ObjPtr.Get();
		bool bInAll = true;
		for (int32 i = 0; i < Collections.Num(); ++i)
		{
			if (i == SmallestIndex) continue;
			if (!Collections[i]->Objects.Contains(Object))
			{
				bInAll = false;
				break;
			}
		}
		if (bInAll) Result.Add(Object);
	}
	return MoveTemp(Result);
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByAllTags(const FGameplayTagContainer& Tags)
{
	TArray<AActor*> Result;
	for (UObject* Object : GetObjectsByAllTags(Tags))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return MoveTemp(Result);
}

int32 UNDynamicRefSubsystem::GetCountByAllTags(const FGameplayTagContainer& Tags)
{
	return GetObjectsByAllTags(Tags).Num();
}