// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefSubsystem.h"

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
	FastCollection[DynamicRef].Add(InObject);
	OnAdded.Broadcast(DynamicRef, InObject);
}

void UNDynamicRefSubsystem::RemoveObject(const ENDynamicRef DynamicRef, UObject* InObject)
{
	FastCollection[DynamicRef].Remove(InObject);
	OnRemoved.Broadcast(DynamicRef, InObject);
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjects(const ENDynamicRef DynamicRef)
{
	return FastCollection[DynamicRef].GetObjects();
}

TArray<UObject*> UNDynamicRefSubsystem::GetObjectsByName(FName Name)
{
	if (NamedCollection.Contains(Name))
	{
		return NamedCollection[Name].GetObjects();
	}
	return TArray<UObject*>();
}

AActor* UNDynamicRefSubsystem::GetFirstActor(const ENDynamicRef DynamicRef)
{
	for (UObject* Object : GetObjects(DynamicRef))
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) return Actor;
	}
	return nullptr;
}

AActor* UNDynamicRefSubsystem::GetFirstActorByName(FName Name)
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
	FNDynamicRefCollection& Collection = FastCollection[DynamicRef];
	if (!Collection.HasObjects()) return nullptr;
	return Collection.Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetFirstObjectUnsafe(const ENDynamicRef DynamicRef)
{
	return FastCollection[DynamicRef].Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetLastObject(const ENDynamicRef DynamicRef)
{
	FNDynamicRefCollection& Collection = FastCollection[DynamicRef];
	if (!Collection.HasObjects()) return nullptr;
	return Collection.Objects.Last();
}

UObject* UNDynamicRefSubsystem::GetLastObjectUnsafe(const ENDynamicRef DynamicRef)
{
	return FastCollection[DynamicRef].Objects.Last();
}

void UNDynamicRefSubsystem::AddObjectByName(const FName Name, UObject* InObject)
{
	if (NamedCollection.Contains(Name))
	{
		NamedCollection[Name].Add(InObject);
	}
	else
	{
		NamedCollection.Add(Name, FNDynamicRefCollection());
		NamedCollection[Name].Add(InObject);
	}
	OnAddedByName.Broadcast(Name, InObject);
}

void UNDynamicRefSubsystem::RemoveObjectByName(const FName Name, UObject* InObject)
{
	if (NamedCollection.Contains(Name))
	{
		NamedCollection[Name].Remove(InObject);
		OnRemovedByName.Broadcast(Name, InObject);
	}
}

TArray<AActor*> UNDynamicRefSubsystem::GetActors(const ENDynamicRef DynamicRef)
{
	TArray<AActor*> Result;
	for (UObject* Object : FastCollection[DynamicRef].GetObjects())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor) Result.Add(Actor);
	}
	return MoveTemp(Result);
}

TArray<AActor*> UNDynamicRefSubsystem::GetActorsByName(FName Name)
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
	return FastCollection[DynamicRef].Objects.Num();
}

int32 UNDynamicRefSubsystem::GetCountByName(FName Name)
{
	if (!NamedCollection.Contains(Name)) return 0;
	return NamedCollection[Name].Objects.Num();
}


UObject* UNDynamicRefSubsystem::GetFirstObjectByName(const FName Name)
{
	if (NamedCollection.Contains(Name))
	{
		return NamedCollection[Name].Objects[0];
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetFirstObjectByNameUnsafe(const FName Name)
{
	return NamedCollection[Name].Objects[0];
}

AActor* UNDynamicRefSubsystem::GetLastActor(const ENDynamicRef DynamicRef)
{
	TArray<UObject*> Objects = GetObjects(DynamicRef);
	const int ObjectsCount = Objects.Num();
	for (int i = ObjectsCount - 1; i >= 0; i--)
	{
		AActor* Actor = Cast<AActor>(Objects[i]);
		if (Actor) return Actor;
	}
	return nullptr;
}

AActor* UNDynamicRefSubsystem::GetLastActorByName(FName Name)
{
	TArray<UObject*> Objects = GetObjectsByName(Name);
	const int ObjectsCount = Objects.Num();
	for (int i = ObjectsCount - 1; i >= 0; i--)
	{
		AActor* Actor = Cast<AActor>(Objects[i]);
		if (Actor) return Actor;
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetLastObjectByName(const FName Name)
{
	if (NamedCollection.Contains(Name))
	{
		return NamedCollection[Name].Objects.Last();
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetLastObjectByNameUnsafe(const FName Name)
{
	return NamedCollection[Name].Objects.Last();
}
