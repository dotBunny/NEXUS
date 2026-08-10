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
	UPROPERTY(EditAnywhere)
	bool bIncludeLandscapes = true;

	/**
	 * When true, Mesh Terrain sections contribute to bounds.
	 * @note Needs its own opt-in because the editor represents a Mesh Partition terrain as transient actors, which
	 *       every other path here skips. Without this a cell whose floor is a Mesh Terrain gets bounds that omit it.
	 * @remark ActorIgnoreTags cannot exclude a Mesh Partition terrain — its actors are regenerated on every build, so
	 *         a tag placed on one does not survive. This flag is the only control over it.
	 */
	UPROPERTY(EditAnywhere)
	bool bIncludeMeshTerrains = true;

	/** Actors carrying any of these tags are excluded from the bounds calculation. */
	UPROPERTY(EditAnywhere)
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
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};