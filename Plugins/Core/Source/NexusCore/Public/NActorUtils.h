// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

/**
 * A collection of utility methods for working with actors.
 */
class NEXUSCORE_API FNActorUtils
{
public:
	/**
	 * Find the RootComponent (USceneComponent) on a Default Object.
	 * @param ActorClass The target class to search for the root component.
	 * @note This has a flaw when navigating through the CDO of a BP generated class. The first found USceneComponent
	 *		 will be treated as the root component. When combined with the NActorPool system, that found components scale
	 *		 is then used as the base scale for the actor. The point here is to do your actor-wide scaling on the root
	 *		 component.
	 */
	static USceneComponent* GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass);
};