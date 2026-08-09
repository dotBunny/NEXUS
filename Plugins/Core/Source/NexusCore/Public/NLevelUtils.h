// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LevelInstance/LevelInstanceInterface.h"

class ALevelInstance;

/**
 * Filter criteria consumed by FNLevelUtils::DetermineLevelBounds when deciding which of a level's actors contribute.
 */
struct NEXUSCORE_API FNLevelBoundsFilter
{
	/** Any actor carrying one of these tags is ignored. */
	TArray<FName> ActorIgnoreTags;

	/** When true, editor-only actors contribute to the bounds. */
	bool bIncludeEditorOnly = false;

	/** When true, actors without collision also contribute to the bounds. */
	bool bIncludeNonColliding = false;

	/** When true, transient actors also contribute to the bounds. */
	bool bIncludeTransientActors = false;

	/**
	 * When true, actors carrying terrain geometry contribute even though they are transient.
	 * @note Narrower than bIncludeTransientActors, which admits every transient actor. Mesh Partition represents an
	 *       authored terrain in the editor as transient APreviewSection actors, so without this a level whose floor
	 *       is a terrain produces bounds that omit it entirely.
	 */
	bool bIncludeTerrain = false;
};

/**
 * A collection of native utility methods for working with levels and level instances.
 *
 * These helpers are only accessible from C++ code and are not exposed to Blueprints.
 * For Blueprint-callable equivalents see UNLevelLibrary.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/level-utils/">FNLevelUtils</a>
 */
class NEXUSCORE_API FNLevelUtils
{
public:
	/**
	 * Walk up the attachment hierarchy of the supplied actor to find its owning level instance.
	 * @param Actor The actor to trace back to a level instance.
	 * @return The level instance the actor belongs to, or nullptr if the actor is not owned by one.
	 */
	static ILevelInstanceInterface* GetActorLevelInstance(const AActor* Actor);

	/**
	 * Convenience accessor that resolves the owning level instance via the component's owning actor.
	 * @param ActorComponent The component whose owner's level instance should be returned.
	 * @return The level instance the component's owner belongs to, or nullptr if there isn't one.
	 */
	FORCEINLINE static ILevelInstanceInterface* GetActorComponentLevelInstance(const UActorComponent* ActorComponent)
	{
		const AActor* Owner = ActorComponent->GetOwner();
		if (!Owner) return nullptr;
		return GetActorLevelInstance(Owner);
	};

	/**
	 * Enumerates the names of all maps reachable from the supplied search paths.
	 * @param SearchPaths A list of content-root relative paths (e.g. "/Game/Maps") to scan for maps.
	 * @return The short asset name of every map found under the search paths, one entry per map asset (not the full package path).
	 */
	static TArray<FString> GetAllMapNames(TArray<FString> SearchPaths);

	/**
	 * Calculates an axis-aligned bounding box that encompasses all relevant actors in a level.
	 * @param InLevel The level whose contents should be considered.
	 * @param OutBounds The calculated bounds; reset on entry and grown by each included actor.
	 * @param OutIgnoredActors Populated with the actors that were skipped during the calculation. Prefill it to
	 *        exclude actors the caller has already ruled out.
	 * @param Filter Criteria deciding which actors contribute (see FNLevelBoundsFilter).
	 */
	static void DetermineLevelBounds(ULevel* InLevel, FBox& OutBounds, TArray<const AActor*>& OutIgnoredActors,
		const FNLevelBoundsFilter& Filter);
};