// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INActorPoolItem.h"
#include "NPooledActorBase.generated.h"

/**
 * A specialized AActor base-class designed to work seamlessly with a FNActorPool.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/pooled-actor-base/">ANPooledActorBase</a>
 */
UCLASS(Abstract, ClassGroup = "NEXUS", DisplayName = "NEXUS | Pooled Actor Base", Meta = (ShortTooltip = "An actor base which support utilizing an NActorPool."))
class NEXUSACTORPOOLS_API ANPooledActorBase : public AActor, public INActorPoolItem
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActorPoolDelegate);
	GENERATED_BODY()

	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	virtual void FellOutOfWorld(const UDamageType& dmgType) override
	{
		ReturnToActorPool();
	}

	virtual void OnCreatedByActorPool() override
	{
		INActorPoolItem::OnCreatedByActorPool();
		OnCreatedByActorPoolEvent.Broadcast();
	};

	virtual void OnReleasedFromActorPool() override
	{
		INActorPoolItem::OnReleasedFromActorPool();
		OnReleasedFromActorPoolEvent.Broadcast();
	};

	virtual void OnReturnToActorPool() override
	{
		INActorPoolItem::OnReturnToActorPool();
		OnReturnToActorPoolEvent.Broadcast();
	};

	virtual void OnSpawnedFromActorPool() override
	{
		INActorPoolItem::OnSpawnedFromActorPool();
		OnSpawnedFromActorPoolEvent.Broadcast();
	};

public:
	/** Event broadcast when the Actor has been created by its Actor Pool. */
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnCreatedByActorPoolEvent;

	/** Event broadcast when the Actor has been spawned from its Actor Pool. */
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnSpawnedFromActorPoolEvent;

	/** Event broadcast when the Actor has been returned to its Actor Pool. */
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnReturnToActorPoolEvent;

	/** Event broadcast when the Actor has been released from its Actor Pool. */
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnReleasedFromActorPoolEvent;
};