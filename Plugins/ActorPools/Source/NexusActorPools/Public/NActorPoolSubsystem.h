// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NActorPool.h"
#include "NActorPoolsMinimal.h"
#include "NActorPoolsSettings.h"
#include "NActorPoolSubsystem.generated.h"

class UNActorPoolObject;
class UNActorPoolSet;
class UNActorPoolSpawnerComponent;

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
	 * @note If the returned actor does not belong in a pool the UNActorPoolsSettings::UnknownBehaviour is applied.
	 * @param Actor The target actor to return to a pool.
	 * @return true/false if the Actor was returned to a pool.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Return Actor", Category = "NEXUS|Actor Pools",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-subsystem/#return-actor"))
	bool ReturnActor(AActor* Actor);

	void RegisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent);
	void UnregisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent);

	//~UTickableWorldSubsystem
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const final override;
	
	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Conditional)
	
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
	 * @remark Meant for native code only to ensure efficiency.
	 */
	OnActorPoolAddedDelegate OnActorPoolAdded;

	/**
	 * Get an array of all the Actor Pools.
	 * @return An array of raw pointers to all the known FNActorPools
	 * @remark This is not meant to be used often and is more for debugging purposes.
	 */
	TArray<FNActorPool*> GetAllPools() const;
	
	/**
	 * Adds default settings for a specific AActor class.
	 * @param ActorClass The class of the AActor to add default settings for.
	 * @param Settings The default settings to apply.
	 * @return True if default settings were added, false if settings already exist for the AActor class.
	 */
	bool AddDefaultSettings(TSubclassOf<AActor> ActorClass, const FNActorPoolSettings& Settings);
	
	/**
	 * Updates default settings for a specific AActor class.
	 * @param ActorClass The class of the AActor to update default settings for.
	 * @param Settings The new default settings to apply.
	 * @return True if default settings were updated, false if no settings were found for the AActor class.
	 */
	bool UpdateDefaultSettings(TSubclassOf<AActor> ActorClass, const FNActorPoolSettings& Settings);
	
	/**
	 * Removes default settings for a specific AActor class.
	 * @param ActorClass The class of the AActor to remove default settings for.
	 * @return True if default settings were removed, false if no settings were found for the AActor class.
	 */
	bool RemoveDefaultSettings(TSubclassOf<AActor> ActorClass);
	
	/**
	 * Checks if default settings are registered for a specific AActor class.
	 * @param ActorClass The class of the AActor to check for default settings.
	 * @return True if default settings are registered for the AActor class, false otherwise.
	 */
	bool HasDefaultSettings(TSubclassOf<AActor> ActorClass) const;

	/**
	 * Returns any registered default settings for a specific AActor, if none are available will access the global UNActorPoolsSettings defaults.
	 * @param ActorClass The class of the AActor which you would like to query for.
	 * @return The default settings for the specified AActor class, or the global defaults if none are registered.
	 */
	FNActorPoolSettings GetDefaultSettings(TSubclassOf<AActor> ActorClass) const;
	
private:
	void AddTickableActorPool(FNActorPool* ActorPool);
	void RemoveTickableActorPool(FNActorPool* ActorPool);
	bool HasTickableActorPool(FNActorPool* ActorPool) const;

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<UClass*, TUniquePtr<FNActorPool>> ActorPools;
	TArray<FNActorPool*> TickableActorPools;
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray<UNActorPoolSpawnerComponent*> TickableSpawners;
	
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FNActorPoolSettings> DefaultSettings;	

	bool bHasTickableActorPools;
	bool bHasTickableSpawners;
	ENActorPoolUnknownBehaviour UnknownBehaviour = ENActorPoolUnknownBehaviour::Destroy;

};

template <typename T>
T* UNActorPoolSubsystem::GetActor(const TSubclassOf<AActor> ActorClass)
{
	if (!ActorPools.Contains(ActorClass))
	{
		if (HasDefaultSettings(ActorClass))
		{
			const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass, GetDefaultSettings(ActorClass)));
			UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool in GetActor for %p|%s (%s) using the registered default settings, raising the total pool count to %i."),
				ActorClass.Get(), *ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
			OnActorPoolAdded.Broadcast(NewPool.Get());
			return Cast<T>(NewPool->Get());
		}
		const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass));
		UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool in GetActor for %p|%s (%s), raising the total pool count to %i."),
			ActorClass.Get(), *ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
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
		if (HasDefaultSettings(ActorClass))
		{
			const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass, GetDefaultSettings(ActorClass)));
			UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool via SpawnActor for %p|%s (%s) using the registered default settings, raising the total pool count to %i."),
				ActorClass.Get(), *ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
			OnActorPoolAdded.Broadcast(NewPool.Get());
			return Cast<T>(NewPool->Spawn(Position, Rotation));
		}
		
		const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass));
		UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool via SpawnActor for %p|%s (%s), raising the total pool count to %i."),
			ActorClass.Get(), *ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		OnActorPoolAdded.Broadcast(NewPool.Get());
		return Cast<T>(NewPool->Spawn(Position, Rotation));
	}
	return Cast<T>((ActorPools.Find(ActorClass)->Get())->Spawn(Position, Rotation));
}