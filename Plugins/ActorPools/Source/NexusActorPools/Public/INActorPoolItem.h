// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPoolSettings.h"
#include "NActorPoolsSettings.h"
#include "INActorPoolItem.generated.h"

class FNActorPool;

/**
 * The operational state of an Actor.
 */
UENUM(BlueprintType)
enum ENActorOperationalState : uint8
{
	AOS_Undefined		UMETA(DisplayName = "Undefined"),
	AOS_Created			UMETA(DisplayName = "Created"),
	AOS_Enabled			UMETA(DisplayName = "Enabled"),
	AOS_Disabled		UMETA(DisplayName = "Disabled"),
	AOS_Destroyed		UMETA(DisplayName = "Destroyed")
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActorOperationalStateChangedDelegate, TEnumAsByte<ENActorOperationalState> OldState, TEnumAsByte<ENActorOperationalState> NewState);

/**
 * An interface to add support to an Actor allowing for it to be pooled more effectively with the ActorPoolSystem.
 */
UINTERFACE(BlueprintType)
class NEXUSACTORPOOLS_API UNActorPoolItem : public UInterface
{
	GENERATED_BODY()
};

/**
 * An interface to add support to an Actor allowing for it to be pooled more effectively with the ActorPoolSystem.
 * @note Any scaling should be done on the RootComponent of the Actor for it to be effectively used when pooling.
 */
class NEXUSACTORPOOLS_API INActorPoolItem
{
	GENERATED_BODY()

public:
	
	void InitializeActorPoolItem(FNActorPool* OwnerPool);

	/**
	 * Is the given Actor attached to an Actor Pool?
	 * @return true/false if properly attached to a pool
	 */
	bool IsAttachedToActorPool() const;

	/**
	 * Return the given Actor to its known Actor Pool.
	 */
	bool ReturnToActorPool();

	/**
	 * Get the ActorPoolSettings used to determine how the ActorPool should be setup for this Actor.
	 */
	virtual FNActorPoolSettings& GetActorPoolSettings() {
		static FNActorPoolSettings DefaultSettings = UNActorPoolsSettings::Get()->DefaultSettings;
		return DefaultSettings;
	};

	/**
	 * Called on the Actor when it has been created by an Actor Pool.
	 */
	virtual void OnCreatedByActorPool() { SetActorOperationalState(AOS_Created); };

	/**
	 * Called after the Actor has been placed back in the Actor Pool, and its settings have been applied.
	 */
	virtual void OnReturnToActorPool() { SetActorOperationalState(AOS_Disabled); };

	/**
	 * Called after the Actor has been spawned from the Actor Pool, and its settings have been applied.
	 */
	virtual void OnSpawnedFromActorPool() { SetActorOperationalState(AOS_Enabled); };

	/**
	 * Called during the deferred construction process for the Actor.
	 */
	virtual void OnDeferredConstruction() { };

	/**
	 * Set the ActorOperationalState of the INActorPoolItem (Actor) calling the change delegate as needed.
	 * @note There are very few use cases for invoking this method, think twice before using it.
	 * @note Will ignore if NewState is the same as ActorOperationalState.
	 * @param NewState The state to change too.
	 */
	void SetActorOperationalState(ENActorOperationalState NewState);
	
	/**
	 * Delegate fired after a transition has occurred in the operational state.
	 * @note Multicast delegates are heavy and should only be used when necessary.
	 */
	FOnActorOperationalStateChangedDelegate OnActorOperationalStateChanged;
	
private:
	/**
	 * Was this actor created by an ActorPool?
	 */
	bool bIsAttachedToActorPool = false;

	/**
	 * Does this actor require a unique construction path.
	 * @note This will trigger OnDeferredConstruction.

	 */
	bool bRequiresDeferredConstruction = false;
	
	/**
	 * A reference to the ActorPool that created the Actor.
	 */
	FNActorPool* OwningActorPool = nullptr;

	/**
	 * The known operational state of the Actor.
	 */
	ENActorOperationalState ActorOperationalState = AOS_Undefined;
};