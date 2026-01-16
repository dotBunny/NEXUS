// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NActorPool.h"
#include "NActorPoolsMinimal.h"
#include "NActorPoolsSettings.h"
#include "NActorPoolSubsystem.generated.h"

class UNActorPoolSet;
class UNActorPoolSpawnerComponent;

USTRUCT(BlueprintType)
struct FNActorPoolStats
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	UClass* Class;
	
	UPROPERTY(BlueprintReadOnly)
	int32 In = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Out = 0;
};

/**
 * A centralized management system that provides UWorld-specific access to AActor pooling functionality, acting as the primary interface for creating, managing, and accessing multiple FNActorPools.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/">UNActorPoolSubsystem</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Actor Pool Subsystem")
class NEXUSACTORPOOLS_API UNActorPoolSubsystem : public UTickableWorldSubsystem
{
	friend class FNActorPool;
	
	GENERATED_BODY()
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNActorPoolSubsystem, true)

	DECLARE_MULTICAST_DELEGATE_OneParam( OnActorPoolAddedDelegate, FNActorPool* );
	
public:

	/**
	 * Gets an actor from a given pool, creating a pool as necessary.
	 * @note This does not trigger any events on the given actor, it does not activate them in any way.
	 * @param ActorClass The class of the actor which you would like to get from the actor pool.
	 * @param ReturnedActor The returned actor.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actor", Category = "NEXUS|Actor Pools",
		meta = (DeterminesOutputType = "ActorClass", DynamicOutputParam = "ReturnedActor",
		DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#get-actor"))
	void GetActor(TSubclassOf<AActor> ActorClass, AActor*& ReturnedActor);

	/**
	 * Gets an actor from a given pool, creating a pool as necessary.
	 * @note This does not trigger any events on the given actor, it does not activate them in any way.
	 * @param ActorClass The class of the actor which you would like to get from the actor pool.
	 * @return The returned actor.
	 */
	template<typename T>
	T* GetActor(TSubclassOf<AActor> ActorClass);

	/**
	* Spawns an actor from a given pool, creating a pool as necessary.
	* @param ActorClass The class of the actor which you would like to get from the actor pool.
	* @param Position The world position to spawn the actor at.
	* @param Rotation The world rotation to apply to the spawned actor.
	* @param SpawnedActor The returned actor.
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Spawn Actor", Category = "NEXUS|Actor Pools",
		meta = (DeterminesOutputType = "ActorClass", DynamicOutputParam = "SpawnedActor",
		DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#spawn-actor"))
	void SpawnActor(TSubclassOf<AActor> ActorClass, FVector Position, FRotator Rotation, AActor*& SpawnedActor);

	/**
	* Spawns an actor from a given pool, creating a pool as necessary.
	* @param ActorClass The class of the actor which you would like to get from the actor pool.
	* @param Position The world position to spawn the actor at.
	* @param Rotation The world rotation to apply to the spawned actor.
	* @return SpawnedActor The spawned actor.
	*/
	template<typename T>
	T* SpawnActor(TSubclassOf<AActor> ActorClass, FVector Position, FRotator Rotation);

	/**
	 * Attempts to return an Actor to its owning pool.
	 * @note If the returned actor does not belong in a pool (and UNActorPoolsSettings::bCreatePoolForUnknownActors is false), it may be destroyed, in that case it will return true.
	 * @param Actor The target actor to return to a pool.
	 * @return true/false if the Actor was returned to a pool.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Return Actor", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#return-actor"))
	bool ReturnActor(AActor* Actor);

	void RegisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent);
	void UnregisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent);

	//~UTickableWorldSubsystem
	virtual void Deinitialize() final override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) final override;
	virtual bool IsTickable() const final override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual void Tick(float DeltaTime) final override;
	//End UTickableWorldSubsystem

	/**
	 * Create an actor pool for the provided Actor class; if one does not already exist.
	 * @param ActorClass The class of the actor which you would like to create a pool for.
	 * @param Settings  The settings to apply to the created pool.
	 * @return true/false if a new pool was created.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Create Actor Pool", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#create-actor-pool"))
	bool CreateActorPool(TSubclassOf<AActor> ActorClass, FNActorPoolSettings Settings);

	/**
	 * Does the given Actor class have a pool already created?
	 * @param ActorClass The class of the actor which you would like to check for a pool.
	 * @return true/false if a pool already exists.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Has Actor Pool", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#has-actor-pool"))
	bool HasActorPool(const TSubclassOf<AActor>& ActorClass) const { return ActorPools.Contains(ActorClass); }

	/**
	 * Apply a preconfigured ActorPoolSet, creating the defined pools.
	 * @param ActorPoolSet  The ActorPoolSet to evaluate.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Apply ActorPoolSet", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#apply-actorpoolset"))
	void ApplyActorPoolSet(UNActorPoolSet* ActorPoolSet);

	/**
	 * Get the pointer to the actor pool itself for a given Actor class.
	 * @param ActorClass The class of the actor which you would like to access a pool for.
	 * @return The pointer to the pool, or nullptr if it was not found.
	 */
	FNActorPool* GetActorPool(const TSubclassOf<AActor> ActorClass) const
	{
		return ActorPools.Find(ActorClass)->Get();
	}

	/**
	 * Event triggered when a new pool is added to the UNActorPoolSubsystem.
	 */
	OnActorPoolAddedDelegate OnActorPoolAdded;

	/**
	 * Get an array of all the Actor Pools.
	 * @return An array of UniquePtr to all the known FNActorPools
	 * @remark This is not meant to be used often and is more for debugging purposes.
	 */
	TArray<FNActorPool*> GetAllPools() const
	{
		TArray<FNActorPool*> ReturnPools;
		ReturnPools.Reserve(ActorPools.Num());
		
		for ( auto Pair = ActorPools.CreateConstIterator(); Pair; ++Pair )
		{
			ReturnPools.Add(Pair.Value().Get());
		}
		return MoveTemp(ReturnPools);
	}

private:
	void AddTickableActorPool(FNActorPool* ActorPool);
	void RemoveTickableActorPool(FNActorPool* ActorPool);
	bool HasTickableActorPool(FNActorPool* ActorPool) const;

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<UClass*, TUniquePtr<FNActorPool>> ActorPools;
	TArray<FNActorPool*> TickableActorPools;
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray<UNActorPoolSpawnerComponent*> TickableSpawners;
	
	bool bHasTickableActorPools;
	bool bHasTickableSpawners;
	ENActorPoolUnknownBehaviour UnknownBehaviour = ENActorPoolUnknownBehaviour::Destroy;

};

template <typename T>
T* UNActorPoolSubsystem::GetActor(const TSubclassOf<AActor> ActorClass)
{
	if (!ActorPools.Contains(ActorClass))
	{
		const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass));
		UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool in GetActor for %s (%s), raising the total pool count to %i."),
			*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		OnActorPoolAdded.Broadcast(NewPool.Get());
		return Cast<T>(NewPool->Get());
	}
	return Cast<T>((*ActorPools.Find(ActorClass))->Get());
}

template <typename T>
T* UNActorPoolSubsystem::SpawnActor(const TSubclassOf<AActor> ActorClass, const FVector Position, const FRotator Rotation)
{
	if (!ActorPools.Contains(ActorClass))
	{
		const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass));
		UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool via SpawnActor for %s (%s), raising the total pool count to %i."),
			*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		OnActorPoolAdded.Broadcast(NewPool.Get());
		return Cast<T>(NewPool->Spawn(Position, Rotation));
	}
	return Cast<T>((ActorPools.Find(ActorClass)->Get())->Spawn(Position, Rotation));
}