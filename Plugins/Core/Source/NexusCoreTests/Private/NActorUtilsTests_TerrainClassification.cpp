// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NActorUtils.h"
#include "Macros/NTestMacros.h"

/**
 * These pin the class names the terrain classification matches on.
 *
 * The matching is a string comparison against types this module deliberately does not link — Landscape, and
 * MeshPartition, an experimental engine plugin that a project may not even have enabled. That keeps NexusCore free of
 * the dependency, at the cost of a classification that an engine upgrade can silently invalidate: rename a section
 * actor and a level's entire floor quietly stops counting as terrain, with a plausible-looking bounds and hull to
 * show for it. Epic has already renamed this family once (MegaMesh to MeshPartition).
 *
 * So these tests are less about the logic, which is trivial, than about turning that rename into a build failure.
 */

N_TEST_HIGH(FNActorUtilsTests_TerrainClassification_MeshPartitionSections,
	"NEXUS::UnitTests::NCore::FNActorUtils::TerrainClassification::MeshPartitionSections",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("The editor's transient preview section is terrain."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("PreviewSection")));
	CHECK_MESSAGE(TEXT("The cooked compiled section is terrain."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("CompiledSection")));

	// A working copy of the region being sculpted, duplicating geometry a preview section already describes, and
	// changing on every mouse move. Counting it would double up the geometry and hold the settle gate open.
	CHECK_FALSE_MESSAGE(TEXT("The interactive section is not treated as terrain."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("InteractiveSection")));

	// Exact, not prefix — otherwise an unrelated type whose name merely begins the same way would be swept in.
	CHECK_FALSE_MESSAGE(TEXT("A name merely starting with a section name is not a section."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("PreviewSectionProxy")));
	CHECK_FALSE_MESSAGE(TEXT("An ordinary actor is not a section."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("StaticMeshActor")));
}

N_TEST_HIGH(FNActorUtilsTests_TerrainClassification_Primitives,
	"NEXUS::UnitTests::NCore::FNActorUtils::TerrainClassification::Primitives",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Landscape is prefix matched because its render, heightfield-collision and mesh-collision components are all
	// separate classes sharing it.
	CHECK_MESSAGE(TEXT("The landscape heightfield collision component is a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(TEXT("LandscapeHeightfieldCollisionComponent")));
	CHECK_MESSAGE(TEXT("The landscape mesh collision component is a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(TEXT("LandscapeMeshCollisionComponent")));
	CHECK_MESSAGE(TEXT("The landscape render component is a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(TEXT("LandscapeComponent")));

	CHECK_MESSAGE(TEXT("The Mesh Partition collision component is a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(TEXT("MeshPartitionCollisionComponent")));

	CHECK_FALSE_MESSAGE(TEXT("An ordinary static mesh component is not a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(TEXT("StaticMeshComponent")));
	CHECK_FALSE_MESSAGE(TEXT("An empty name is not a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(FString()));
}

N_TEST_HIGH(FNActorUtilsTests_TerrainClassification_Authoring,
	"NEXUS::UnitTests::NCore::FNActorUtils::TerrainClassification::Authoring",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("The Mesh Partition definition actor is authoring apparatus."),
		FNActorUtils::IsTerrainAuthoringClassName(TEXT("MeshPartition")));
	CHECK_MESSAGE(TEXT("A modifier actor is authoring apparatus."),
		FNActorUtils::IsTerrainAuthoringClassName(TEXT("ModifierActor")));

	// The distinction that matters: authoring apparatus must never also read as geometry, or excluding it from a
	// bounds calculation would be undone by admitting it as terrain.
	CHECK_FALSE_MESSAGE(TEXT("Authoring apparatus is not a terrain section."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("MeshPartition")));
	CHECK_FALSE_MESSAGE(TEXT("A modifier actor is not a terrain section."),
		FNActorUtils::IsTerrainSectionClassName(TEXT("ModifierActor")));
	CHECK_FALSE_MESSAGE(TEXT("A terrain section is not authoring apparatus."),
		FNActorUtils::IsTerrainAuthoringClassName(TEXT("PreviewSection")));

	// Exact match: MeshPartitionCollisionComponent shares the prefix but is geometry, not apparatus.
	CHECK_FALSE_MESSAGE(TEXT("The Mesh Partition collision component is not authoring apparatus."),
		FNActorUtils::IsTerrainAuthoringClassName(TEXT("MeshPartitionCollisionComponent")));
}

N_TEST_MEDIUM(FNActorUtilsTests_TerrainClassification_Landscape,
	"NEXUS::UnitTests::NCore::FNActorUtils::TerrainClassification::Landscape",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("A landscape component is landscape."),
		FNActorUtils::IsLandscapeClassName(TEXT("LandscapeHeightfieldCollisionComponent")));

	// Landscape has to be separable from the rest of terrain: it is the one with no geometry to extract, so it takes
	// the sampling path instead of the factory.
	CHECK_FALSE_MESSAGE(TEXT("A Mesh Partition section is not landscape."),
		FNActorUtils::IsLandscapeClassName(TEXT("MeshPartitionCollisionComponent")));
	CHECK_FALSE_MESSAGE(TEXT("An ordinary static mesh component is not landscape."),
		FNActorUtils::IsLandscapeClassName(TEXT("StaticMeshComponent")));
}

N_TEST_MEDIUM(FNActorUtilsTests_TerrainClassification_MeshTerrain,
	"NEXUS::UnitTests::NCore::FNActorUtils::TerrainClassification::MeshTerrain",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("A Mesh Partition collision component is mesh terrain."),
		FNActorUtils::IsMeshTerrainPrimitiveClassName(TEXT("MeshPartitionCollisionComponent")));

	// The mirror of the landscape test above: the two representations answer to separate generation flags, so a
	// predicate that admitted both would make one of those flags unable to exclude anything.
	CHECK_FALSE_MESSAGE(TEXT("A landscape component is not mesh terrain."),
		FNActorUtils::IsMeshTerrainPrimitiveClassName(TEXT("LandscapeHeightfieldCollisionComponent")));
	CHECK_FALSE_MESSAGE(TEXT("An ordinary static mesh component is not mesh terrain."),
		FNActorUtils::IsMeshTerrainPrimitiveClassName(TEXT("StaticMeshComponent")));

	// Both halves have to keep agreeing with the whole, since IsTerrainActor is now composed of them.
	CHECK_MESSAGE(TEXT("Either representation is a terrain primitive."),
		FNActorUtils::IsTerrainPrimitiveClassName(TEXT("MeshPartitionCollisionComponent"))
		&& FNActorUtils::IsTerrainPrimitiveClassName(TEXT("LandscapeHeightfieldCollisionComponent")));
}

N_TEST_MEDIUM(FNActorUtilsTests_TerrainClassification_NullActors,
	"NEXUS::UnitTests::NCore::FNActorUtils::TerrainClassification::NullActors",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every one of these walks a level's actor array, which routinely holds nulls.
	CHECK_FALSE_MESSAGE(TEXT("A null actor is not terrain."), FNActorUtils::IsTerrainActor(nullptr));
	CHECK_FALSE_MESSAGE(TEXT("A null actor is not authoring apparatus."), FNActorUtils::IsTerrainAuthoringActor(nullptr));
	CHECK_FALSE_MESSAGE(TEXT("A null actor is not landscape."), FNActorUtils::IsLandscapeActor(nullptr));
	CHECK_FALSE_MESSAGE(TEXT("A null actor is not mesh terrain."), FNActorUtils::IsMeshTerrainActor(nullptr));
	CHECK_FALSE_MESSAGE(TEXT("A null primitive is not a terrain primitive."), FNActorUtils::IsTerrainPrimitive(nullptr));
	CHECK_FALSE_MESSAGE(TEXT("A null primitive has no built geometry."), FNActorUtils::HasBuiltGeometry(nullptr));

	const FBox Box = FNActorUtils::GetBuiltComponentsBoundingBox(nullptr, true);
	CHECK_FALSE_MESSAGE(TEXT("A null actor yields an invalid box rather than a point at the origin."), Box.IsValid != 0);
}

#endif //WITH_TESTS
