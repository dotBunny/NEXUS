// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPoolSettings.h"
#include "Types/NToggle.h"

/**
 * A runtime-unique controlling object that maintains a pool of spawned actors.
 */
class NEXUSACTORPOOLS_API FNActorPool
{
public:
	/**
	 * Create an ActorPool.
	 *
	 * @param TargetWorld The world where to create the actors.
	 * @param ActorClass The class of actor to be created and pooled.
	 */
	FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass);
	/**
	 * Create an ActorPool.
	 *
	 * @param TargetWorld The world where to create the actors.
	 * @param ActorClass The class of actor to be created and pooled.
	 * @param InActorPoolSetting Settings to apply to the ActorPool, overriding the ActorPoolItem definition, and defaults.
	 */
	FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass, const FNActorPoolSettings& InActorPoolSetting);

	/**
	 * Clear and destroy the contents of the ActorPool, both actors in and out of the pool.
	 * @param bForceDestroy Should the Actors be forcibly destroyed when the pool is cleared.
	 */
	void Clear(const bool bForceDestroy = false);

	/**
	* Fill the pool with actors up to the set minimum number.
	*/
	void Fill();

	/**
	* Warm the pool with actors, regardless of settings.
	*/
	void Warm(int Count);

	/**
	 * Get an actor from the pool, not invoking any of the events attached with regular spawning of an actor.
	 */
	AActor* Get();

	/**
	 * Return the given actor to the pool.
	 *
	 * @note This will add any actor provided to the pool, regardless if it came from the pool originally.
	 * @param Actor The target actor to be returned.
	 */
	bool Return(AActor* Actor);

	/**
	 * Spawn an actor at a given location, triggering all events associated.
	 *
	 * @param Position The world location where to spawn the actor.
	 * @param Rotation The world rotation of the spawned actor.
	 */
	AActor* Spawn(const FVector& Position, const FRotator& Rotation);

	/**
	 * A managed-tick for the ActorPool, which creates delayed number of actors and other maintenance.
	 */
	void Tick();

	/**
	 * Apply settings to ActorPool.
	 *
	 * @param InNewSettings Reference to an ActorPoolSettings to apply.
	 */
	void UpdateSettings(const FNActorPoolSettings& InNewSettings);

	/**
	 * Does the ActorPool's Template implement the IActorPoolItem interface?
	 */
	bool DoesSupportInterface() const { return bImplementsInterface; }

	/**
	 * Get the calculated half height of the ActorPool's Template.
	 */
	double GetHalfHeight() const { return HalfHeight; };
	/**
	 * Get the calculated half height of the ActorPool's Template as an offset vector.
	 */
	FVector GetHalfHeightOffset() const { return HalfHeightOffset; };

	int GetInCount() const { return InActors.Num(); };
	int GetOutCount() const { return OutActors.Num(); };
	FNActorPoolSettings& GetSettings() { return Settings; };
	
	/**
	 * Does the ActorPool's Template have a cached half height?
	 * @note This is useful for finding the midpoint to spawn characters.
	 */
	bool HasHalfHeight() const { return bHasHalfHeight; };

	/**
	 * Does the ActorPool have any actors currently in the pool?
	 * @note This does not count actors that have been spawned and are considered 'out'.
	 */
	bool IsEmpty() const { return InActors.IsEmpty(); };
	

private:

	const FTransform DefaultTransform = FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector::One());
	static FNActorPoolSettings DefaultSettings;

	void PreInitialize(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass);
	void PostInitialize();

	/**
	 * An array of Actors considered pool in the ActorPool, which can be used for Get/Spawn requests.
	 */
	TArray<TObjectPtr<AActor>> InActors;
	/**
	 * An array of Actors that have been spawned from the ActorPool and are not considered 'in'.
	 */
	TArray<TObjectPtr<AActor>> OutActors;

	TSubclassOf<AActor> Template;
	FVector TemplateScale = FVector(1, 1, 1);

	bool bHasHalfHeight;
	double HalfHeight;
	FVector HalfHeightOffset;

	bool ApplyStrategy();
	void CreateActor();

	FORCEINLINE void ApplySpawnState(AActor* Actor, const FVector& InPosition, const FRotator& InRotation) const;
	FORCEINLINE void ApplyReturnState(AActor* Actor) const;

	UWorld* World;
	FNActorPoolSettings Settings;

#if WITH_EDITOR
	FString Name;
	static int ActorPoolTicket;
#endif
	bool bImplementsInterface = false;

	ENToggle SpawnPhysicsSimulation = T_Default;

	ECollisionEnabled::Type SpawnPhysicsCollisionSettings;
};