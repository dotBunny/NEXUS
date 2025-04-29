// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LevelInstance/LevelInstanceInterface.h"

class ALevelInstance;
/**
 * A collection of utility methods for working with levels.
 */
class NEXUSCORE_API FNLevelUtils
{
public:
	static ILevelInstanceInterface* GetActorLevelInstance(const AActor* Actor);

	FORCEINLINE static ILevelInstanceInterface* GetActorComponentLevelInstance(const UActorComponent* ActorComponent)
	{
		return GetActorLevelInstance(ActorComponent->GetOwner());
	};

	static TArray<FString> GetAllMapNames(TArray<FString> SearchPaths);
};