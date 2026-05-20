// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefSubsystem.h"

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
		UE_LOG(LogNexusDynamicRefs, Warning, TEXT("Requested count of NDR_None, returning -1."));
		return -1;
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
	if (Collection != nullptr)
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