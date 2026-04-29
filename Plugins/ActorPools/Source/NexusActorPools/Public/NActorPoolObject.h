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
	GENERATED_BODY()

public:

	/**
	 * Factory method that creates a new UNActorPoolObject and links it to the provided native pool.
	 * @param Outer The UObject owner of the new wrapper.
	 * @param ActorPool The native pool this wrapper should reflect.
	 * @return A newly allocated, linked UNActorPoolObject.
	 */
	static UNActorPoolObject* Create(UObject* Outer, FNActorPool* ActorPool)
	{
		UNActorPoolObject* Object = NewObject<UNActorPoolObject>(Outer, StaticClass(), NAME_None, RF_Transient);
		Object->Link(ActorPool);
		return Object;
	}

	/**
	 * Bind this wrapper to a native FNActorPool and cache display metadata from it.
	 * @param NewPool The pool to link.
	 */
	void Link(FNActorPool* NewPool)
	{
		this->Pool = NewPool;

		FString ClassNameOriginal = NewPool->GetTemplate()->GetFName().ToString();
		ClassNameOriginal.RemoveFromEnd("_C");

		this->ClassName = FText::FromString(ClassNameOriginal);
	}

	/** @return The number of Actors currently in the pool (available to spawn), or -1 if unlinked. */
	UFUNCTION(BlueprintCallable)
	int32 GetInCount() const
	{
		if (Pool == nullptr) return -1;
		return Pool->GetInCount();
	}

	/** @return The number of Actors currently out of the pool (spawned in the world), or -1 if unlinked. */
	UFUNCTION(BlueprintCallable)
	int32 GetOutCount() const
	{
		if (Pool == nullptr) return -1;
		return Pool->GetOutCount();
	}

	/** @return An Actor from the pool without spawning it into the world, or nullptr if none available. */
	UFUNCTION(BlueprintCallable)
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
	UFUNCTION(BlueprintCallable)
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
	UFUNCTION(BlueprintCallable)
	bool Return(AActor* Actor) const
	{
		if (Pool == nullptr) return false;
		return Pool->Return(Actor);
	}

	/** @return The Actor class template this pool produces. */
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AActor> GetTemplate()
	{
		if (Pool == nullptr) return nullptr;
		return Pool->GetTemplate();
	};

	/** @return The display name of the Actor class this pool produces (with any trailing "_C" stripped). */
	UFUNCTION(BlueprintCallable)
	FText GetClassName() const
	{
		return ClassName;
	}

	/** @return The UWorld the underlying pool is bound to, or nullptr if unlinked. */
	UFUNCTION(BlueprintCallable)
	UWorld* GetPoolWorld() const
	{
		if (Pool == nullptr) return nullptr;
		return Pool->GetWorld();
	}

	/** @return true if the pool's Actor class implements INActorPoolItem. */
	UFUNCTION(BlueprintCallable)
	bool ImplementsPoolItemInterface() const
	{
		if (Pool == nullptr) return false;
		return Pool->ImplementsPoolItemInterface();
	}

	/** @return true if the pool's flags include InvokeUFunctions. */
	UFUNCTION(BlueprintCallable)
	bool HasInvokeUFunctionFlag() const
	{
		if (Pool == nullptr) return false;
		return Pool->HasInvokeUFunctionFlag();
	}

	/** @return A human-readable description of the pool's configuration. */
	UFUNCTION(BlueprintCallable)
	FText GetDescription() const
	{
		if (Pool == nullptr) return FText::FromString("Pool == nullptr");
		return Pool->GetDescription();
	}

private:
	/** The native Actor Pool this wrapper reflects. */
	FNActorPool* Pool;

	/** Cached display name of the pool's Actor template. */
	FText ClassName;
};