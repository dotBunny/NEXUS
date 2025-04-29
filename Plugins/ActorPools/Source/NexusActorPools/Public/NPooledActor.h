// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INActorPoolItem.h"
#include "NPooledActor.generated.h"

/**
 * An actor base which support utilizing an ActorPool.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "An actor base which support utilizing an NActorPool."))
class NEXUSACTORPOOLS_API ANPooledActor : public AActor, public INActorPoolItem
{
	GENERATED_BODY()

	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	virtual void FellOutOfWorld(const UDamageType& dmgType) override
	{
		ReturnToActorPool();
	}
};