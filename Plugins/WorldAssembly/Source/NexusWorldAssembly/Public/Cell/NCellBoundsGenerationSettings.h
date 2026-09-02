// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NArrayUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "NCellBoundsGenerationSettings.generated.h"

/**
 * Settings controlling how a cell's world-space bounds are computed from its level content.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellBoundsGenerationSettings
{
	GENERATED_BODY()

	/** When true, bounds are recomputed automatically whenever the cell is saved. */
	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	/** When true, actors without collision still contribute to bounds. */
	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	/** When true, editor-only actors contribute to bounds. */
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	/**
	 * When true, landscapes contribute to bounds.
	 * @note Split from the Mesh Terrain flag below: the two are different kinds of actor with different reasons to be
	 *       refused. A landscape is an ordinary saved actor whose surface has to be sampled rather than read, where a
	 *       Mesh Terrain is transient and rebuilt.
	 */
	UPROPERTY(EditAnywhere, DisplayName = "Include Landscapes (EXPERIMENTAL)")
	bool bIncludeLandscapes = false;

	/**
	 * When true, Mesh Terrain sections contribute to bounds.
	 * @note Needs its own opt-in because the editor represents a Mesh Partition terrain as transient actors, which
	 *       every other path here skips. Without this a cell whose floor is a Mesh Terrain gets bounds that omit it.
	 * @remark ActorIgnoreTags cannot exclude a Mesh Partition terrain — its actors are regenerated on every build, so
	 *         a tag placed on one does not survive. This flag is the only control over it.
	 */
	UPROPERTY(EditAnywhere, DisplayName = "Include Mesh Terrains (EXPERIMENTAL)")
	bool bIncludeMeshTerrains = false;

	/**
	 * When true, foliage actors contribute to the bounds.
	 * @note A setting rather than an unconditional refusal, unlike the PCG partition container the bounds drops outright.
	 *       Foliage is scenery in nearly every case, but it is scenery somebody placed and may legitimately want
	 *       accounted for — so the answer is the author's, exactly as it is for landscape.
	 * @note Landscape grass is not foliage under this flag: it belongs to its landscape and answers to
	 *       bIncludeLandscapes. See FNActorUtils::IsFoliageActor.
	 */
	UPROPERTY(EditAnywhere, DisplayName = "Include Foliage")
	bool bIncludeFoliage = false;

	/**
	 * Actors, and individual components, carrying any of these tags are excluded from the bounds calculation.
	 *
	 * One list covering both levels rather than a pair: a cell markup tag means the same thing wherever it is placed,
	 * so a tag an author adds here is honored on an actor and on a component without having to be added twice.
	 * @note Component granularity matters because the actor is frequently not the author's unit of choice — a
	 *       generator writes its whole result onto one container actor, so an actor tag takes all of it, where tagging
	 *       the spawner's components individually keeps some of what it produced and leaves the rest out.
	 */
	UPROPERTY(EditAnywhere, DisplayName = "Ignore Tags",
		meta=(ToolTip="Tags that exclude an actor, or an individual component of one, from the bounds calculation."))
	TArray<FName> ActorIgnoreTags = { NEXUS::WorldAssembly::ActorTags::CellIgnore, NEXUS::WorldAssembly::ActorTags::CellBoundsIgnore };

	// TODO: Padding to surfaces ? character height
	/** @return true if all fields match structurally; used when diffing live vs. side-car cell data. */
	bool Equals(const FNCellBoundsGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly
		&& bIncludeLandscapes == Other.bIncludeLandscapes
		&& bIncludeMeshTerrains == Other.bIncludeMeshTerrains
		&& bIncludeFoliage == Other.bIncludeFoliage
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};