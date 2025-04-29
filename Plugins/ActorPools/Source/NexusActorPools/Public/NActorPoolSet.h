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

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ActorClass;

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