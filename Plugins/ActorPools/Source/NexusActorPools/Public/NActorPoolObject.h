// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPool.h"
#include "NActorPoolObject.generated.h"

UCLASS(BlueprintType)
class NEXUSACTORPOOLS_API UNActorPoolObject : public UObject
{
	GENERATED_BODY()

public:

	static UNActorPoolObject* Create(UObject* Outer, FNActorPool* ActorPool)
	{
		UNActorPoolObject* Object = NewObject<UNActorPoolObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->Link(ActorPool);
		return Object;
	}
	
	void Link(FNActorPool* NewPool)
	{
		this->Pool = NewPool;
		
		FString ClassNameOriginal = NewPool->GetTemplate()->GetFName().ToString();
		ClassNameOriginal.RemoveFromEnd("_C");
		
		this->ClassName = FText::FromString(ClassNameOriginal);
	}
	
	UFUNCTION(BlueprintCallable)
	int GetInCount() const
	{
		if (Pool == nullptr) return -1;
		return Pool->GetInCount();
	}
	
	UFUNCTION(BlueprintCallable)
	int GetOutCount() const
	{
		if (Pool == nullptr) return -1;
		return Pool->GetOutCount();
	}
	
	UFUNCTION(BlueprintCallable)
	AActor* GetActor() const
	{
		if (Pool == nullptr) return nullptr;
		return Pool->Get();
	}
	
	UFUNCTION(BlueprintCallable)
	AActor* Spawn(const FVector& Position, const FRotator& Rotation) const
	{
		if (Pool == nullptr) return nullptr;
		return Pool->Spawn(Position, Rotation);
	}
	
	UFUNCTION(BlueprintCallable)
	bool Return(AActor* Actor) const
	{
		if (Pool == nullptr) return false;
		return Pool->Return(Actor);
	}
	
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AActor> GetTemplate()
	{
		if (Pool == nullptr) return nullptr;
		return Pool->GetTemplate();
	};
	
	UFUNCTION(BlueprintCallable)
	FText GetClassName() const
	{
		return ClassName;
	}
	
	UFUNCTION(BlueprintCallable)
	UWorld* GetPoolWorld() const
	{
		if (Pool == nullptr) return nullptr;
		return Pool->GetWorld();
	}
	
private:	
	FNActorPool* Pool;
	FText ClassName;
};