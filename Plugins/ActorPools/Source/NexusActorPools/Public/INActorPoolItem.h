// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPool.h"
#include "NActorPoolSettings.h"
#include "NActorPoolsSettings.h"
#include "INActorPoolItem.generated.h"

class FNActorPool;

/**
 * The operational state of an Actor managed by an FNActorPool.
 */
UENUM(BlueprintType)
enum class ENActorOperationalState : uint8
{
	/** No state has been assigned yet. */
	Undefined = 0,
	/** The actor has just been created by the pool but has not yet been spawned into the world. */
	Created = 1,
	/** The actor is active in the world (spawned from the pool). */
	Enabled	= 2,
	/** The actor has been returned to the pool and is inactive. */
	Disabled = 3,
	/** The actor has been destroyed by the pool. */
	Destroyed = 4,
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActorOperationalStateChangedDelegate, const ENActorOperationalState OldState, const ENActorOperationalState NewState);

/**
 * An interface to add support to an Actor allowing for it to be pooled more effectively with the ActorPoolSystem.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-item/">UNActorPoolItem</a>
 */
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class NEXUSACTORPOOLS_API UNActorPoolItem : public UInterface
{
	GENERATED_BODY()
};

/**
 * An interface to add support to an Actor allowing for it to be pooled more effectively with the ActorPoolSystem.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-item/">INActorPoolItem</a>
 */
class NEXUSACTORPOOLS_API INActorPoolItem
{
	GENERATED_BODY()

public:

	/**
	 * Bind this Actor to the given FNActorPool instance.
	 * @param OwnerPool The ActorPool that created/owns this Actor.
	 * @note Called internally by FNActorPool; there is rarely a reason to invoke this directly.
	 */
	void InitializeActorPoolItem(FNActorPool* OwnerPool);

	/**
	 * Is the given Actor attached to an Actor Pool?
	 * @return true/false if properly attached to a pool
	 */
	bool IsAttachedToActorPool() const;

	/**
	 * Return the given Actor to its known Actor Pool.
	 * @return true if the Actor was successfully returned, false if it was not attached to a pool.
	 */
	bool ReturnToActorPool();

	/** Get the ActorPoolSettings used to determine how the ActorPool should be set up for this Actor. */
	virtual const FNActorPoolSettings& GetActorPoolSettings() {
		if (OwningActorPool != nullptr)
		{
			return OwningActorPool->GetSettings();
		}
		return UNActorPoolsSettings::Get()->DefaultSettings;
	};

	/** Called on the Actor when it has been created by an Actor Pool. */
	virtual void OnCreatedByActorPool() { SetActorOperationalState(ENActorOperationalState::Created); };

	/** Called on the Actor when it has been destroyed by an Actor Pool. */
	virtual void OnDestroyedByActorPool() { SetActorOperationalState(ENActorOperationalState::Destroyed); };

	/** Called after the Actor has been placed back in the Actor Pool, and its settings have been applied. */
	virtual void OnReturnToActorPool() { SetActorOperationalState(ENActorOperationalState::Disabled); };

	/** Called after the Actor has been spawned from the Actor Pool, and its settings have been applied. */
	virtual void OnSpawnedFromActorPool() { SetActorOperationalState(ENActorOperationalState::Enabled); };

	/**
	 * Called during the deferred construction process for the Actor.
	 * @note Default implementation is a no-op; override to perform per-spawn construction work before the Actor is finalized.
	 */
	virtual void OnDeferredConstruction()
	{
		// Implementations should fill this out as needed.
	};

	/**
	 * Set the ActorOperationalState of the INActorPoolItem (Actor) calling the change delegate as needed.
	 * @note There are very few use cases for invoking this method, think twice before using it.
	 * @note Will ignore if NewState is the same as ActorOperationalState.
	 * @param NewState The state to change too.
	 * @return Was a change made?
	 */
	bool SetActorOperationalState(ENActorOperationalState NewState);

	/**
	 * Returns the current state of the INActorPoolItem.
	 * @return The current state.
	 */
	ENActorOperationalState GetCurrentActorOperationalState() const { return CurrentActorOperationalState; };

	/**
	 * Returns the previous state of the INActorPoolItem.
	 * @return The previous state.
	 */
	ENActorOperationalState GetPreviousActorOperationalState() const { return PreviousActorOperationalState; };
	

	/**
	 * Delegate fired after a transition has occurred in the operational state.
	 * @note Multicast delegates are heavy and should only be used when necessary.
	 */
	FOnActorOperationalStateChangedDelegate OnActorOperationalStateChanged;
	
private:
	/** Was this actor created by an ActorPool? */
	bool bIsAttachedToActorPool = false;
	
	/** A reference to the ActorPool that created the Actor. */
	FNActorPool* OwningActorPool = nullptr;

	/** The known operational state of the Actor. */
	ENActorOperationalState CurrentActorOperationalState = ENActorOperationalState::Undefined;

	/** The previous operational state of the Actor. */
	ENActorOperationalState PreviousActorOperationalState = ENActorOperationalState::Undefined;
};