// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NDynamicRef.h"
#include "NDynamicRefSubsystem.h"
#include "NDynamicRefObject.generated.h"

UCLASS(BlueprintType)
class NEXUSDYNAMICREFS_API UNDynamicRefObject : public UObject
{
	GENERATED_BODY()

public:
	
	// ADD DELEGATE TO BIND TO THAT WILL GET CALLED WHEN A CHANGE HAPPENS
	
	static UNDynamicRefObject* Create(UObject* Outer, const FName Name, const FNDynamicRefCollection& Collection, UWorld* World)
	{
		UNDynamicRefObject* Object = NewObject<UNDynamicRefObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->TargetName = Name;
		return Object;
	};

	static UNDynamicRefObject* Create(UObject* Outer, const ENDynamicRef DynamicRef, const FNDynamicRefCollection& Collection, UWorld* World)
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
	
	TArray<TWeakPtr<UObject>> TargetObjects;
	

protected:
	
	
	
private:
	FName TargetName;
	ENDynamicRef TargetDynamicRef;
};