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

void UNDynamicRefSubsystem::AddFastReference(const ENDynamicRef InType, UObject* InObject)
{
	FastCollection[InType].Add(InObject);
}

void UNDynamicRefSubsystem::RemoveFastReference(const ENDynamicRef InType, UObject* InObject)
{
	FastCollection[InType].Remove(InObject);
}

TArray<UObject*> UNDynamicRefSubsystem::GetFastReferences(const ENDynamicRef InType)
{
	return FastCollection[InType].GetObjects();
}


UObject* UNDynamicRefSubsystem::GetFastReferenceFirst(const ENDynamicRef InType)
{
	FNDynamicRefCollection& Collection = FastCollection[InType];
	if (!Collection.HasObjects()) return nullptr;
	return Collection.Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetFastReferenceFirstUnsafe(const ENDynamicRef InType)
{
	return FastCollection[InType].Objects[0].Get();
}

UObject* UNDynamicRefSubsystem::GetFastReferenceLast(const ENDynamicRef InType)
{
	FNDynamicRefCollection& Collection = FastCollection[InType];
	if (!Collection.HasObjects()) return nullptr;
	return Collection.Objects.Last();
}

UObject* UNDynamicRefSubsystem::GetFastReferenceLastUnsafe(const ENDynamicRef InType)
{
	return FastCollection[InType].Objects.Last();
}

void UNDynamicRefSubsystem::AddNamedReference(const FName Name, UObject* InObject)
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
}

void UNDynamicRefSubsystem::RemoveNamedReference(const FName Name, UObject* InObject)
{
	if (NamedCollection.Contains(Name))
	{
		NamedCollection[Name].Remove(InObject);
	}
}

TArray<UObject*> UNDynamicRefSubsystem::GetNamedReferences(const FName Name)
{
	if (NamedCollection.Contains(Name))
	{
		return NamedCollection[Name].GetObjects();
	}
	return TArray<UObject*>();
}

UObject* UNDynamicRefSubsystem::GetNamedReferenceFirst(const FName Name)
{
	if (NamedCollection.Contains(Name))
	{
		return NamedCollection[Name].Objects[0];
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetNamedReferenceFirstUnsafe(const FName Name)
{
	return NamedCollection[Name].Objects[0];
}

UObject* UNDynamicRefSubsystem::GetNamedReferenceLast(const FName Name)
{
	if (NamedCollection.Contains(Name))
	{
		return NamedCollection[Name].Objects.Last();
	}
	return nullptr;
}

UObject* UNDynamicRefSubsystem::GetNamedReferenceLastUnsafe(const FName Name)
{
	return NamedCollection[Name].Objects.Last();
}
