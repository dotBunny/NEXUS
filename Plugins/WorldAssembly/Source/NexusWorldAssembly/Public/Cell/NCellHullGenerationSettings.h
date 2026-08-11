// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Chaos/Convex.h"
#include "NWorldAssemblyMinimal.h"
#include "NCellHullGenerationSettings.generated.h"

/**
 * Convex-hull construction algorithm — kept in sync with Chaos::FConvexBuilder::EBuildMethod.
 */
UENUM(BlueprintType)
enum class ENullBuildMethod : uint8
{
	Default = 0,
	Original = 1,
	ConvexHull3 = 2,
	ConvexHull3Simplified = 3,
};

/**
 * Settings controlling how a cell's convex hull is generated from its level content.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellHullGenerationSettings
{
	GENERATED_BODY()

	/** When true, the hull is recomputed automatically whenever the cell is saved. */
	UPROPERTY(EditAnywhere)
	bool bCalculateOnSave = true;

	/** When true, the hull may retain concave regions instead of being forced to a strict convex shape. */
	UPROPERTY(EditAnywhere)
	bool bAllowNonConvex = false;

	/** When true, actors without collision still contribute to the hull. */
	UPROPERTY(EditAnywhere)
	bool bIncludeNonColliding = false;

	/** When true, editor-only actors contribute to the hull. */
	UPROPERTY(EditAnywhere)
	bool bIncludeEditorOnly = false;

	/**
	 * When true, landscapes contribute to the hull.
	 * @note Split from the Mesh Terrain flag below: the two are different kinds of actor with different reasons to be
	 *       refused. A landscape is an ordinary saved actor whose surface has to be sampled rather than read, where a
	 *       Mesh Terrain is transient and rebuilt.
	 */
	UPROPERTY(EditAnywhere, DisplayName = "Include Landscapes (EXPERIMENTAL)")
	bool bIncludeLandscapes = false;

	/**
	 * When true, Mesh Terrain sections contribute to the hull.
	 * @note Needs its own opt-in because the editor represents a Mesh Partition terrain as transient actors, which the
	 *       hull's actor filter skips. Without this a cell whose floor is a Mesh Terrain gets a hull with no floor in
	 *       it, and the assembly penetration tests that consume the hull let other cells sink through it.
	 * @remark ActorIgnoreTags cannot exclude a Mesh Partition terrain — its actors are regenerated on every build, so
	 *         a tag placed on one does not survive. This flag is the only control over it.
	 */
	UPROPERTY(EditAnywhere, DisplayName = "Include Mesh Terrains (EXPERIMENTAL)")
	bool bIncludeMeshTerrains = false;

	/**
	 * Grid size, in world units, that terrain vertices are thinned onto before the hull is built. 0 keeps every one.
	 *
	 * Reads as how much slack the envelope is allowed rather than how detailed it is: a convex hull is decided by its
	 * extreme points alone, so thinning barely moves the resulting shape — it shifts each supporting plane outward by
	 * at most about this distance. At the default, the hull sits within a metre of the one every vertex would give.
	 * @note Only terrain is thinned. Authored geometry arrives as a handful of collision primitives, but a terrain
	 *       section hands over its entire surface — four sections measured at 251,001 vertices each, a million points
	 *       into a convex build that is superlinear in them.
	 * @note Applies to generation only, and assumes the convex build that follows it. A hull hand-edited into a
	 *       concave shape afterwards is unaffected; but were generation itself ever made concave, snapping outward
	 *       would push a concave surface into the void it is meant to bound and would have to change with it.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin="0", Units="cm"))
	float TerrainSimplificationGridSize = 100.f;

	/** Algorithm used to build the convex hull. */
	UPROPERTY(VisibleAnywhere)
	ENullBuildMethod BuildMethod = ENullBuildMethod::Original;

	/** Actors carrying any of these tags are excluded from the hull calculation. */
	UPROPERTY(EditAnywhere)
	TArray<FName> ActorIgnoreTags = { NEXUS::WorldAssembly::ActorTags::CellIgnore, NEXUS::WorldAssembly::ActorTags::CellHullIgnore };

	/** @return The Chaos enum matching BuildMethod. */
	Chaos::FConvexBuilder::EBuildMethod GetChaosBuildMethod() const
	{
		switch (BuildMethod)
		{
			using enum ENullBuildMethod;
		case Default:
			return Chaos::FConvexBuilder::EBuildMethod::Default;
		case Original:
			return Chaos::FConvexBuilder::EBuildMethod::Original;
		case ConvexHull3:
			return Chaos::FConvexBuilder::EBuildMethod::ConvexHull3;
		case ConvexHull3Simplified:
			return Chaos::FConvexBuilder::EBuildMethod::ConvexHull3Simplified;
		default:
			return Chaos::FConvexBuilder::EBuildMethod::Default;
		}
	}

	// TODO: Padding to surfaces ? character height
	/** @return true if all fields match structurally; used when diffing live vs. side-car cell data. */
	bool Equals(const FNCellHullGenerationSettings& Other) const
	{
		return bCalculateOnSave == Other.bCalculateOnSave
		&& bAllowNonConvex == Other.bAllowNonConvex
		&& bIncludeNonColliding == Other.bIncludeNonColliding
		&& bIncludeEditorOnly == Other.bIncludeEditorOnly && BuildMethod == Other.BuildMethod
		&& bIncludeLandscapes == Other.bIncludeLandscapes
		&& bIncludeMeshTerrains == Other.bIncludeMeshTerrains
		&& TerrainSimplificationGridSize == Other.TerrainSimplificationGridSize
		&& FNArrayUtils::IsSameOrderedValues(ActorIgnoreTags, Other.ActorIgnoreTags);
	}
};