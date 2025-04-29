// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NActorPool.h"
#include "NCoreMinimal.h"
#include "NActorPoolSubsystem.generated.h"

class UNActorPoolSpawnerComponent;

UCLASS()
class NEXUSACTORPOOLS_API UNActorPoolSubsystem : public UTickableWorldSubsystem
{
	friend class FNActorPool;

	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNActorPoolSubsystem, true)

public:

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools",
		meta = (DeterminesOutputType = "ActorClass", DynamicOutputParam = "ReturnedActor"))
	/**
	 * Gets an actor from a given pool, creating a pool as necessary.
	 *
	 * @notes This does not trigger any events on the given actor, it does not activate them in anyway.
	 * @param ActorClass The class of the actor which you would like to get from the actor pool.
	 * @return ReturnedActor The returned actor.
	 */
	void GetActor(TSubclassOf<AActor> ActorClass, AActor*& ReturnedActor);

	template<typename T>
	T* GetActor(TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools",
		meta = (DeterminesOutputType = "ActorClass", DynamicOutputParam = "SpawnedActor"))
	void SpawnActor(TSubclassOf<AActor> ActorClass, FVector Position, FRotator Rotation, AActor*& SpawnedActor);

	template<typename T>
	T* SpawnActor(TSubclassOf<AActor> ActorClass, FVector Position, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	bool ReturnActor(AActor* Actor);

	void RegisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent);
	void UnregisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent);

	void SetDestroyUnknownReturnedActors(bool ShouldDestroy);

	//~UTickableWorldSubsystem
	virtual void Deinitialize() final override;
	virtual bool IsTickable() const final override;
	virtual void Tick(float DeltaTime) final override;
	//End UTickableWorldSubsystem

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	bool CreateActorPool(TSubclassOf<AActor> ActorClass, FNActorPoolSettings Settings);

	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	bool HasActorPool(const TSubclassOf<AActor>& ActorClass) const { return ActorPools.Contains(ActorClass); }
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor Pools")
	void ApplyActorPoolSet(UNActorPoolSet* ActorPoolSet);

	FNActorPool* GetActorPool(const TSubclassOf<AActor> ActorClass) const { return ActorPools.FindRef(ActorClass); }

private:
	void AddTickableActorPool(FNActorPool* ActorPool);
	void RemoveTickableActorPool(FNActorPool* ActorPool);

	TMap<UClass*, FNActorPool*> ActorPools;
	TArray<FNActorPool*> TickableActorPools;
	TArray<UNActorPoolSpawnerComponent*> TickableSpawners;
	
	bool bHasTickableActorPools;
	bool bHasTickableSpawners;
	bool bDestroyUnknownReturnedActors = true;

};

template <typename T>
T* UNActorPoolSubsystem::GetActor(const TSubclassOf<AActor> ActorClass)
{
	if (!ActorPools.Contains(ActorClass))
	{
		// #RawPointer - I did try to have this as a UObject, however I was not able to resolve behavioral differences
		// with the TSubclassOf<AActor> when looking up pools on UNActorPoolSubsystem.
		FNActorPool* NewPool = new FNActorPool(GetWorld(), ActorClass);
		ActorPools.Add(ActorClass, NewPool);
		N_LOG(Log,
			TEXT("[UNActorPoolSubsystem::GetActor] Creating a new pool in GetActor for %s (%s), raising the total pool count to %i."),
			*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		return Cast<T>(NewPool->Get());
	}
	return Cast<T>((*ActorPools.Find(ActorClass))->Get());
}

template <typename T>
T* UNActorPoolSubsystem::SpawnActor(const TSubclassOf<AActor> ActorClass, const FVector Position, const FRotator Rotation)
{
	if (!ActorPools.Contains(ActorClass))
	{
		// #RawPointer - I did try to have this as a UObject, however I was not able to resolve behavioral differences
		// with the TSubclassOf<AActor> when looking up pools on UNActorPoolSubsystem.
		FNActorPool* NewPool = new FNActorPool(GetWorld(), ActorClass);
		ActorPools.Add(ActorClass, NewPool);
		N_LOG(Log,
			TEXT("[UNActorPoolSubsystem::SpawnActor] Creating a new pool via SpawnActor for %s (%s), raising the total pool count to %i."),
			*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		return Cast<T>(NewPool->Spawn(Position, Rotation));
	}
	return Cast<T>((ActorPools.FindRef(ActorClass))->Spawn(Position, Rotation));
}