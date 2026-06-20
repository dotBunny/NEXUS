// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPool.h"
#include "NActorPoolObject.generated.h"

/**
 * UObject wrapper around a native FNActorPool, exposing pool operations to Blueprints and UMG bindings.
 */
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | Actor Pool Object")
class NEXUSACTORPOOLS_API UNActorPoolObject : public UObject
{
	friend class FNActorPool;
	
	GENERATED_BODY()

public:

	/**
	 * Factory method that returns the wrapper reflecting the supplied native pool, creating one if it does not already exist.
	 * @param Outer The UObject owner used when a new wrapper is allocated.
	 * @param ActorPool The native pool this wrapper should reflect.
	 * @return The pool's existing wrapper if one is already linked, otherwise a newly allocated, linked UNActorPoolObject.
	 * @note Each FNActorPool may only have a single UNActorPoolObject linked at a time so the pool's destructor has a deterministic back-pointer to clear.
	 * @note An already-linked wrapper is returned unchanged; its cached ClassName/OwningWorld derive from the pool's immutable template/world, so no refresh is needed.
	 */
	static UNActorPoolObject* Create(UObject* Outer, FNActorPool* ActorPool)
	{
		if (ActorPool->LinkedActorPoolObject != nullptr)
		{
			return ActorPool->LinkedActorPoolObject;
		}

		UNActorPoolObject* Object = NewObject<UNActorPoolObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->Link(ActorPool);
		return Object;
	}

	/**
	 * Bind this wrapper to a native FNActorPool and cache display metadata from it.
	 * @param NewPool The pool to link.
	 * @note Also stores a back-pointer on the pool so its destructor can null Pool here, preventing dangling reads after the pool is destroyed.
	 */
	void Link(FNActorPool* NewPool)
	{
		NewPool->LinkedActorPoolObject = this;
		this->Pool = NewPool;
		this->OwningWorld = NewPool->GetWorld();

		FString ClassNameOriginal = NewPool->GetTemplate()->GetFName().ToString();
		ClassNameOriginal.RemoveFromEnd("_C");

		this->ClassName = FText::FromString(ClassNameOriginal);
	}

	/** @return The number of Actors currently in the pool (available to spawn), or -1 if unlinked. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	int32 GetAvailableCount() const
	{
		if (Pool == nullptr) return -1;
		return Pool->GetAvailableCount();
	}

	/** @return The number of Actors currently out of the pool (spawned in the world), or -1 if unlinked. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	int32 GetSpawnedCount() const
	{
		if (Pool == nullptr) return -1;
		return Pool->GetSpawnedCount();
	}

	/** @return An Actor from the pool without spawning it into the world, or nullptr if none available. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	AActor* GetActor() const
	{
		if (Pool == nullptr) return nullptr;
		return Pool->Get();
	}

	/**
	 * Spawn an Actor from the pool at the given transform.
	 * @param Position World position to spawn at.
	 * @param Rotation World rotation to spawn with.
	 * @return The spawned Actor, or nullptr if the pool is exhausted.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	AActor* Spawn(const FVector& Position, const FRotator& Rotation) const
	{
		if (Pool == nullptr) return nullptr;
		return Pool->Spawn(Position, Rotation);
	}

	/**
	 * Return an Actor to the pool.
	 * @param Actor The Actor to return.
	 * @return true if the Actor was successfully returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	bool Return(AActor* Actor) const
	{
		if (Pool == nullptr) return false;
		return Pool->Return(Actor);
	}

	/** @return The Actor class template this pool produces. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	TSubclassOf<AActor> GetTemplate()
	{
		if (Pool == nullptr) return nullptr;
		return Pool->GetTemplate();
	};

	/** @return The display name of the Actor class this pool produces (with any trailing "_C" stripped). */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	FText GetClassName() const
	{
		return ClassName;
	}

	/** @return The UWorld the underlying pool is bound to, or nullptr if unlinked. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	UWorld* GetPoolWorld() const
	{
		if (Pool != nullptr) return Pool->GetWorld();
		return OwningWorld.Get();
	}

	/** @return true if the pool's Actor class implements INActorPoolItem. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	bool ImplementsPoolItemInterface() const
	{
		if (Pool == nullptr) return false;
		return Pool->ImplementsPoolItemInterface();
	}

	/** @return true if the pool's flags include InvokeUFunctions. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	bool ShouldInvokeUFunctions() const
	{
		if (Pool == nullptr) return false;
		return Pool->ShouldInvokeUFunctions();
	}

	/** @return A human-readable description of the pool's configuration. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|ActorPools")
	FText GetDescription() const
	{
		if (Pool == nullptr) return FText::FromString("Pool == nullptr");
		return Pool->GetDescription();
	}
	
	/**
	 * @return true if this wrapper is still linked to a live native FNActorPool.
	 * @note Returns false once the pool's destructor has cleared the back-pointer (e.g. after world teardown), so callers can safely skip work that would otherwise dereference a destroyed pool.
	 */
	bool IsValid() const
	{
		return Pool != nullptr;
	}

private:
	/** The native Actor Pool this wrapper reflects. */
	FNActorPool* Pool = nullptr;

	/** Cached world pointer so GetPoolWorld() remains valid after pool destruction. */
	TWeakObjectPtr<UWorld> OwningWorld;

	/** Cached display name of the pool's Actor template. */
	FText ClassName;
};