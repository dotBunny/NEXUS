// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INActorPoolItem.h"
#include "NPooledActor.generated.h"

/**
 * A specialized AActor base-class designed to work seamlessly with a FNActorPool.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/pooled-actor/">ANPooledActor</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Pooled Actor", Meta = (ShortTooltip = "An actor base which support utilizing an NActorPool."))
class NEXUSACTORPOOLS_API ANPooledActor : public AActor, public INActorPoolItem
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

	virtual void OnDestroyedByActorPool() override
	{
		INActorPoolItem::OnDestroyedByActorPool();
		OnDestroyedByActorPoolEvent.Broadcast();
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
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnCreatedByActorPoolEvent;
	
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnSpawnedFromActorPoolEvent;
	
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnReturnToActorPoolEvent;
	
	UPROPERTY(BlueprintAssignable)
	FOnActorPoolDelegate OnDestroyedByActorPoolEvent;
};