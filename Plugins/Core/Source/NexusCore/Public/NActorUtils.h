// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

/**
 * Filter criteria consumed by FNActorUtils::GetWorldActors when collecting actors from a UWorld.
 */
struct NEXUSCORE_API FNWorldActorFilterSettings
{
	/** When true, actors flagged as editor-only (AActor::IsEditorOnly) are skipped during iteration. */
	bool bExcludeEditorOnly = true;

	/** When true, actors whose AActor::GetActorEnableCollision() returns false are skipped. */
	bool bExcludeNonCollisionEnabledActors = false;

	/**
	 * When true, APlayerStart actors are unconditionally included — they bypass the editor-only, collision, and predicate
	 * filters. Useful when callers need spawn locations even though the player-start actor would otherwise be filtered out.
	 */
	bool bIncludePlayerStarts = false;

	/**
	 * Optional caller-supplied predicate evaluated per actor. Return true to keep the actor, false to exclude it.
	 * @note Owned by value, so safe to assign a temporary lambda. Leave default-constructed (empty) to skip the predicate check entirely.
	 */
	TFunction<bool(const AActor*)> ExclusionFunction;
};

/**
 * A collection of utility methods for working with actors.
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
	 * Find the root USceneComponent template for an Actor class.
	 * @param ActorClass The target class to inspect. A null TSubclassOf is tolerated and returns nullptr.
	 * @return The first USceneComponent template encountered while walking up the Blueprint class chain; or — when no
	 *         UBlueprintGeneratedClass in the chain contributes a scene-component root — the native CDO's RootComponent.
	 *         May still be nullptr for classes that have no native root (e.g. AActor::StaticClass()).
	 * @note Callers that read spawn-time settings off the returned template (scale, BodyInstance flags, etc.) are reading
	 *       the live class template, not a copy — configure actor-wide defaults on the root component itself so that
	 *       systems like NActorPool pick them up.
	 *
	 * @details For Blueprint-generated classes, walks each UBlueprintGeneratedClass in the inheritance chain and returns
	 *          the first USceneComponent found among that class's SCS root-node ComponentTemplates. The walk stops at
	 *          the first non-Blueprint ancestor and falls back to the native CDO's RootComponent. Null SCS nodes and
	 *          non-scene ComponentTemplates are skipped rather than aborting the search, so a child BP that does not
	 *          override its root still resolves to an ancestor BP's SCS-defined root.
	 */
	static USceneComponent* GetRootComponentFromDefaultObject(const TSubclassOf<AActor>& ActorClass);

	/**
	 * Collect every actor in the supplied world that satisfies the provided filter settings.
	 * @param World The world to iterate. A null world yields an empty array.
	 * @param Settings Filter criteria applied to each candidate actor (see FNWorldActorFilterSettings).
	 * @return The set of actors that survived all filtering checks, in iteration order.
	 * @note Null and pending-kill actors are always skipped. APlayerStart actors are short-circuited into the result when
	 *       bIncludePlayerStarts is set, bypassing every other filter. Otherwise, an actor is kept only when it passes
	 *       the editor-only and collision-enabled checks and (when set) the ExclusionFunction predicate.
	 */
	static TArray<AActor*> GetWorldActors(const UWorld* World, const FNWorldActorFilterSettings& Settings);
};