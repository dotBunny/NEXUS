// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefObject.h"

#include "NDynamicRefComponent.h"
#include "NDynamicRefsDeveloperOverlay.h"
#include "NDynamicRefsMinimal.h"
#include "Macros/NValidationMacros.h"

UNDynamicRefObject* UNDynamicRefObject::Create(UObject* Outer, const FName Name)
{
	UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
	Object->Overlay = Cast<UNDynamicRefsDeveloperOverlay>(Outer);
	Object->TargetName = Name;
	Object->TargetDynamicRef = NDR_None;
	return Object;
}

UNDynamicRefObject* UNDynamicRefObject::Create(UObject* Outer, const ENDynamicRef DynamicRef)
{
	UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
	Object->Overlay = Cast<UNDynamicRefsDeveloperOverlay>(Outer);
	Object->TargetDynamicRef = DynamicRef;
	Object->TargetName = NAME_None;
	return Object;
}

void UNDynamicRefObject::AddObject(UObject* Object)
{
	N_VALIDATE_RETURN_VOID(LogNexusDynamicRefs, Object);
	TargetObjects.AddUnique(Object);
	Changed.ExecuteIfBound();
}

void UNDynamicRefObject::RemoveObject(UObject* Object)
{
	if (TargetObjects.Remove(Object) > 0)
	{
		Changed.ExecuteIfBound();
	}
}

int32 UNDynamicRefObject::GetCount()
{
	Compact();
	return TargetObjects.Num();
}

FText UNDynamicRefObject::GetReferenceText() const
{
	if (TargetName != NAME_None)
	{
		return FText::FromName(TargetName);
	}
	return FText::FromString(UNDynamicRefComponent::ToStringSlow(TargetDynamicRef));
}

TArray<TWeakObjectPtr<UObject>>& UNDynamicRefObject::GetObjects()
{
	Compact();
	return TargetObjects;
}

void UNDynamicRefObject::Compact()
{
	TargetObjects.RemoveAll([](const TWeakObjectPtr<UObject>& Object){ return !Object.IsValid(); });
}
