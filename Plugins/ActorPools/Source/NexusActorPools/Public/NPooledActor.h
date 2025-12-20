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
UCLASS(DisplayName = "NEXUS: Pooled Actor", Config = Game, Meta = (ShortTooltip = "An actor base which support utilizing an NActorPool."))
class NEXUSACTORPOOLS_API ANPooledActor : public AActor, public INActorPoolItem
{
	GENERATED_BODY()

	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	virtual void FellOutOfWorld(const UDamageType& dmgType) override
	{
		ReturnToActorPool();
	}
};