// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

/**
 * @class FNActorUtils
 * @brief A collection of utility methods for working with actors.
 *
 * The FNActorUtils class provides static utility functions for common Actor-related
 * operations in Unreal Engine. These functions are designed to simplify working
 * with Actor instances, particularly focusing on component access and manipulation.
 * Unlike UNActorLibrary, these utilities are only accessible from C++ code and not 
 * exposed to Blueprints.
 */
class NEXUSCORE_API FNActorUtils
{
public:
	/**
	 * Find the RootComponent (USceneComponent) on a Default Object.
	 * 
	 * @param ActorClass The target class to search for the root component.
	 * @return The root USceneComponent of the specified Actor class, or nullptr if none is found.
	 * 
	 * @note This has a flaw when navigating through the CDO of a Blueprint-generated class. The first found USceneComponent
	 *		 will be treated as the root component. When combined with the NActorPool system, that found components scale
	 *		 is then used as the base scale for the actor. The point here is to do your actor-wide scaling on the root
	 *		 component.
	 * 
	 * @details For Blueprint-generated classes, this function attempts to find the first USceneComponent in the
	 *          SimpleConstructionScript hierarchy. If the class is not Blueprint-generated or no USceneComponent
	 *          is found in the SCS, it falls back to the default object's root component.
	 */
	static USceneComponent* GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass);
};