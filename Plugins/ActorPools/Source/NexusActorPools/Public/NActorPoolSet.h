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
	 * The Actor Pool's settings.
	 */
	UPROPERTY(EditAnywhere)
	FNActorPoolSettings Settings;
};

/**
 * A UDataAsset that serves as a collection of preconfigured FNActorPoolSettings.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-set/">UNActorPoolSet</a>
 */
UCLASS()
class NEXUSACTORPOOLS_API UNActorPoolSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{ActorClass} ({MinimumActorCount})"))
	TArray<FNActorPoolDefinition> ActorPools;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UNActorPoolSet>> NestedSets;
	
	bool TryGetUniqueSets(TArray<UNActorPoolSet*>& OutActorPoolSets)
	{
		const int AdditionalCount = NestedSets.Num();
		OutActorPoolSets.Reserve(OutActorPoolSets.Num() + AdditionalCount + 1);

		// Add current
		if (!OutActorPoolSets.Contains(this))
		{
			OutActorPoolSets.Add(this);
		}

		// Add children/s
		for (int i = 0; i < AdditionalCount; i++)
		{
			if (!OutActorPoolSets.Contains(NestedSets[i])) // Prevent infinite loop
			{
				NestedSets[i]->TryGetUniqueSets(OutActorPoolSets);
			}
		}

		return OutActorPoolSets.Num() >= 1;
	}
};