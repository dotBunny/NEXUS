// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPoolSettings.h"
#include "Types/NToggle.h"

class UNActorPoolObject;

namespace NEXUS::ActorPools::InvokeMethods
{
	inline const FText Category = FText::FromString("NActor Pool");
	inline const FName OnDeferredConstruction = TEXT("OnDeferredConstruction");
	inline const FName OnCreatedByActorPool = TEXT("OnCreatedByActorPool");
	inline const FName OnReleasedFromActorPool = TEXT("OnReleasedFromActorPool");
	inline const FName OnReturnToActorPool = TEXT("OnReturnToActorPool");
	inline const FName OnSpawnedFromActorPool = TEXT("OnSpawnedFromActorPool");
	
}

/**
 * A runtime-unique controlling object that maintains a pool of spawned actors.
 * @note Not thread-safe, must be used on the game thread due to creating actors.
 */
class NEXUSACTORPOOLS_API FNActorPool
{
	// Allow to test w/ deep access
	friend class UNActorPoolObject;
#if WITH_TESTS	
	friend class FNActorPoolTests_Return_StorageLocation;
	friend class FNActorPoolTests_Create_ActorsAreNotRooted;
	friend class FNActorPoolTests_Clear_ForceDestroyOnRootedActor_WarnsAndSkips;
#endif // WITH_TESTS	
	
public:
	/**
	 * Create an ActorPool.
	 * @param TargetWorld The world where to create the actors.
	 * @param ActorClass The class of actor to be created and pooled.
	 */
	FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass);
	/**
	 * Create an ActorPool.
	 * @param TargetWorld The world where to create the actors.
	 * @param ActorClass The class of actor to be created and pooled.
	 * @param InActorPoolSetting Settings to apply to the ActorPool, overriding the ActorPoolItem definition, and defaults.
	 */
	FNActorPool(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass, const FNActorPoolSettings& InActorPoolSetting);

	/**
	 * Destroy the ActorPool and notify any linked UNActorPoolObject so it can null its back-pointer.
	 * @note Required because UNActorPoolObject holds a raw FNActorPool* that would otherwise dangle when the owning subsystem releases this pool (e.g. on world teardown).
	 */
	~FNActorPool();

	/**
	 * Clear and destroy the contents of the ActorPool, both actors in and out of the pool.
	 * @param bForceDestroy Should the Actors be forcibly destroyed when the pool is cleared?
	 */
	void Clear(const bool bForceDestroy = false);

	/** Fill the pool with actors up to the set minimum number. */
	void Fill();

	/** Warm the pool with actors, regardless of settings. */
	void Prewarm(int32 Count);

	/** Get an actor from the pool, not invoking any of the events attached with regular spawning of an actor. */
	AActor* Get();

	/**
	 * Return the given actor to the pool.
	 * @note Accepts any actor whose class matches the pool's Template, regardless of whether the actor was originally created by this pool. Null pointers are always rejected; mismatched-class actors and actors already in the pool are rejected and logged in non-Shipping builds (the checks are stripped in Shipping for hot-path cost — callers must honour the same contract there).
	 * @param Actor The target actor to be returned.
	 */
	bool Return(AActor* Actor);

	/**
	 * Return every currently spawned ('out') actor back to the pool.
	 * @note Iterates in reverse so each Return() can safely remove from the OutActors array as it goes.
	 * @param bSkipCheck When false (default), the pool must have the ENActorPoolSupportFlags::ReturnAll support flag set or the call is logged and ignored; pass true to bypass that gate and force the return.
	 */
	void ReturnAll(bool bSkipCheck = false);

	/**
	 * Spawn an actor at a given location, triggering all events associated.
	 * @param Position The world location where to spawn the actor.
	 * @param Rotation The world rotation of the spawned actor.
	 */
	AActor* Spawn(const FVector& Position, const FRotator& Rotation);

	/**
	 * A managed-tick for the ActorPool that time-slices warm-up creation across frames.
	 * @return true if the pool still needs ticking; false once it has reached its minimum (or is a stub) and can be unregistered.
	 */
	bool Tick();

	/**
	 * Apply settings to ActorPool.
	 * @param InNewSettings Reference to an ActorPoolSettings to apply.
	 */
	void UpdateSettings(const FNActorPoolSettings& InNewSettings);

	/** Does the ActorPool's Template implement the IActorPoolItem interface? */
	bool ImplementsPoolItemInterface() const { return bImplementsInterface; }

	/** Will the ActorPool attempt to invoke UFUNCTIONs for events? */
	bool ShouldInvokeUFunctions() const { return Settings.HasFlag_InvokeUFunctions(); }

	/** Get the number of AActors currently in the pool. */
	int32 GetAvailableCount() const { return InActors.Num(); };

	/** Get the number of AActors currently out of the pool. */
	int32 GetSpawnedCount() const { return OutActors.Num(); };

	/** Get the settings currently applied to the ActorPool. */
	const FNActorPoolSettings& GetSettings() const { return Settings; };

	/**
	 * Does the ActorPool have any actors currently in the pool?
	 * @note This does not count actors that have been spawned and are considered 'out'.
	 */
	bool IsEmpty() const { return InActors.IsEmpty(); };

	/**
	 * Is the ActorPool a client-side stub for a ServerOnly pool?
	 * @note When true, pool operations (Return, Spawn, Tick, ApplyStrategy, etc.) short-circuit to no-ops so the pool can exist on non-authoritative peers without doing work.
	 */
	bool IsStubMode() const { return bStubMode; };

	/** Get the actor class the ActorPool spawns and pools instances of. */
	TSubclassOf<AActor> GetTemplate() const { return Template; };

	/** Get the world the ActorPool's actors are spawned into. */
	UWorld* GetWorld() const { return World; };


	/**
	 * Build a multi-line human-readable summary of the ActorPool's world, template, strategy, and flags.
	 * @remark Intended for debugging and developer overlays; the format is not stable and should not be parsed.
	 */
	FText GetDescription() const;
	
private:

	void PreInitialize(UWorld* TargetWorld, const TSubclassOf<AActor>& ActorClass);
	void PostInitialize();

	/** An array of Actors considered pooled in the ActorPool, which can be used for Get/Spawn requests. */
	TArray<TObjectPtr<AActor>> InActors;
	/** An array of Actors that have been spawned from the ActorPool and are not considered 'in'. */
	TArray<TObjectPtr<AActor>> OutActors;

	TSubclassOf<AActor> Template;
	
	bool bStubMode = false;

	bool ApplyStrategy();

	/**
	 * Create up to MinimumActorCount on demand when a Fixed* pool's time-sliced warm-up never completed.
	 * @return true if at least one actor was created; false if the pool already holds MinimumActorCount or more.
	 * @note Fixed strategies create nothing through ApplyStrategy's normal path, so this is their only self-heal if the warm-up Tick() was never delivered (e.g. the pool was created in a world that won't tick again).
	 */
	bool WarmUpDeficit();

	/**
	 * Return the out-actor at a known OutActors index back to the pool (recycle fast path).
	 * @param OutIndex Index into OutActors of the actor being recycled; must be in range.
	 * @return true if the actor was recycled; false if the slot held a null reference.
	 * @note Used only by ApplyStrategy's recycle strategies, which already know the index. Skips the pointer re-scan and external-caller contract checks that Return() performs, and uses an ordered RemoveAt to preserve the FIFO/LIFO ordering those strategies rely on.
	 */
	bool ReturnAtIndex(int32 OutIndex);

	/** Shared tail of the return paths: move the actor (already removed from OutActors) into InActors and fire the return callback. */
	void FinalizeReturn(AActor* Actor);

	bool CreateActors(const int32 Count = 1);

	/** Reserve both staging arrays so each can hold the whole pool population, since churn (Get/Spawn/Return) can move every actor onto either side. */
	FORCEINLINE void ReserveForPoolSize(const int32 PoolSize);

	FORCEINLINE bool CreateActor(const FActorSpawnParameters& SpawnInfo);
	
	void ReleaseActor(TObjectPtr<AActor> Actor, bool bForceDestroy) const;

	FORCEINLINE void ApplySpawnState(AActor* Actor, const FVector& InPosition, const FRotator& InRotation) const;
	FORCEINLINE void ApplyReturnState(AActor* Actor) const;

	TObjectPtr<UWorld> World;

	/** The UObject wrapper currently reflecting this pool, if any; cleared in the destructor so the wrapper can detect that the native pool is gone. */
	TObjectPtr<UNActorPoolObject> LinkedActorPoolObject;

	FNActorPoolSettings Settings;

#if WITH_EDITOR
	FString Name;
	static int32 ActorPoolTicket;
#endif // WITH_EDITOR
	bool bImplementsInterface = false;

	
	ENToggle SpawnPhysicsSimulation = ENToggle::Default;
	ECollisionEnabled::Type SpawnPhysicsCollisionSettings;
};