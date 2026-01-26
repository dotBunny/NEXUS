// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NDynamicRef.h"
#include "NDynamicRefComponent.h"
#include "NDynamicRefObject.generated.h"

UCLASS(BlueprintType)
class NEXUSDYNAMICREFS_API UNDynamicRefObject : public UObject
{
	GENERATED_BODY()

	DECLARE_DELEGATE( OnDynamicRefObjectChangeDelegate );
	
public:
	
	// ADD DELEGATE TO BIND TO THAT WILL GET CALLED WHEN A CHANGE HAPPENS
	
	static UNDynamicRefObject* Create(UObject* Outer, const FName Name)
	{
		UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->TargetName = Name;
		return Object;
	};

	static UNDynamicRefObject* Create(UObject* Outer, const ENDynamicRef DynamicRef)
	{
		UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->TargetDynamicRef = DynamicRef;
		return Object;
	};
	
	
	UFUNCTION(BlueprintCallable)
	ENDynamicRef GetDynamicRef() const
	{
		return TargetDynamicRef;
	}
	
	UFUNCTION(BlueprintCallable)
	FName GetTargetName() const
	{
		return TargetName;
	}
	
	UFUNCTION(BlueprintCallable)
	void AddObject(UObject* Object)
	{
		TargetObjects.Add(Object);
		Changed.ExecuteIfBound();
	}
	
	UFUNCTION(BlueprintCallable)
	void RemoveObject(UObject* Object)
	{
		TargetObjects.Remove(Object);
		Changed.ExecuteIfBound();
	}
	
	UFUNCTION(BlueprintCallable)
	int32 GetCount()
	{
		return TargetObjects.Num();
	}
	
	FText GetReferenceText() const
	{
		if (TargetName != NAME_None)
		{
			return FText::FromName(TargetName);
		}
		return FText::FromString(UNDynamicRefComponent::ToStringSlow(TargetDynamicRef));
	}
	
	TArray<TObjectPtr<UObject>>& GetObjects()
	{
		return TargetObjects;
	}
	
	OnDynamicRefObjectChangeDelegate Changed;

private:
	TArray<TObjectPtr<UObject>> TargetObjects;
	FName TargetName;
	ENDynamicRef TargetDynamicRef;
};