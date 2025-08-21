// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPoolSettings.h"
#include "Engine/DataAsset.h"
#include "NActorPoolSet.generated.h"

USTRUCT()
struct NEXUSACTORPOOLS_API FNActorPoolDefinition
{
	GENERATED_BODY()

	/**
	 * The Actor to create a pool for.
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ActorClass;

	/**
	 * The created Actor Pool's settings.
	 */
	UPROPERTY(EditAnywhere)
	FNActorPoolSettings Settings;
};

/**
 * A collection of predefined ActorPools.
 */
UCLASS()
class NEXUSACTORPOOLS_API UNActorPoolSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{ActorClass} ({MinimumActorCount})"))
	TArray<FNActorPoolDefinition> ActorPools;
};