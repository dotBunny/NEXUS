// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

class UPrimitiveComponent;

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

	TArray<FName> WorldCollisionActorIgnoreTags;

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
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/actor-utils/">FNActorUtils</a>
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

	/**
	 * Evaluate a single actor against the supplied filter settings — the same per-actor logic GetWorldActors applies
	 * while iterating a world. Lets callers reactively test one actor (e.g. an actor reported by an editor change
	 * delegate) without re-scanning the whole world.
	 * @param Actor Candidate actor under inspection.
	 * @param Settings Filter criteria to apply (see FNWorldActorFilterSettings).
	 * @return true if the actor would be kept by GetWorldActors under these settings.
	 * @note Null and pending-kill (non-IsValid) actors return false. APlayerStart actors short-circuit to true when
	 *       bIncludePlayerStarts is set, bypassing every other filter — identical to GetWorldActors.
	 */
	static bool PassesFilter(const AActor* Actor, const FNWorldActorFilterSettings& Settings);

	/**
	 * @param ClassName Unprefixed UClass name of a primitive component.
	 * @return true when the name is one this framework treats as carrying terrain geometry.
	 * @remark The string form exists so the matching can be tested without the plugins that define these types, and
	 *         so an engine upgrade that renames one of them fails a test rather than silently classifying a level's
	 *         entire floor as ordinary geometry. Epic has renamed this family once already (MegaMesh to
	 *         MeshPartition), which is precisely the event this guards.
	 */
	static bool IsTerrainPrimitiveClassName(const FString& ClassName);

	/**
	 * @param ClassName Unprefixed UClass name of an actor.
	 * @return true when the name is a terrain section — a built piece of terrain geometry.
	 * @note Matched exactly. The interactive section is deliberately not one: it is a working copy of whatever is
	 *       being sculpted, duplicating geometry a preview section already describes.
	 */
	static bool IsTerrainSectionClassName(const FString& ClassName);

	/**
	 * @param ClassName Unprefixed UClass name of an actor.
	 * @return true when the name is terrain authoring apparatus rather than terrain.
	 */
	static bool IsTerrainAuthoringClassName(const FString& ClassName);

	/**
	 * @param ClassName Unprefixed UClass name of a component.
	 * @return true when the name belongs to the landscape family.
	 */
	static bool IsLandscapeClassName(const FString& ClassName);

	/**
	 * @param ClassName Unprefixed UClass name of a component.
	 * @return true when the name is the Mesh Terrain collision component.
	 * @note The landscape half of IsTerrainPrimitiveClassName has always been separable; this is the other half, so a
	 *       caller can admit one representation without the other.
	 */
	static bool IsMeshTerrainPrimitiveClassName(const FString& ClassName);

	/**
	 * Identify a primitive that carries terrain geometry — a landscape component, or a Mesh Partition section's
	 * collision component.
	 * @param Primitive Primitive component to inspect.
	 * @return true when the component belongs to one of the recognized terrain representations.
	 * @note Matched on class name so NexusCore takes no dependency on the Landscape module, nor on MeshPartition,
	 *       which is an experimental engine plugin that may not be enabled at all.
	 */
	static bool IsTerrainPrimitive(const UPrimitiveComponent* Primitive);

	/**
	 * Identify an actor that carries terrain geometry, by inspecting the primitives it owns.
	 * @param Actor Candidate actor under inspection. A null actor returns false.
	 * @return true when any primitive the actor owns satisfies IsTerrainPrimitive.
	 * @remark Exists so bounds and hull generation can admit terrain that their ordinary filters would drop. Mesh
	 *         Partition represents an authored terrain in the editor as transient APreviewSection actors, and a
	 *         blanket transient skip would silently omit a cell's entire floor.
	 */
	static bool IsTerrainActor(const AActor* Actor);

	/**
	 * Identify an actor that describes how a terrain is built rather than being terrain itself — a Mesh Partition
	 * definition, or one of the modifiers that sculpt it.
	 * @param Actor Candidate actor under inspection. A null actor returns false.
	 * @return true when the actor is terrain authoring apparatus.
	 * @remark These must never contribute to a bounds or hull calculation at any setting. A modifier's bounds are its
	 *         region of influence, which reaches far past the surface it produces — measured against a real level, a
	 *         single modifier's box was larger than every piece of geometry in it put together.
	 */
	static bool IsTerrainAuthoringActor(const AActor* Actor);

	/**
	 * Identify an actor whose terrain is a landscape.
	 * @param Actor Candidate actor under inspection. A null actor returns false.
	 * @return true when the actor owns a landscape primitive.
	 * @remark Worth separating from IsTerrainActor because landscape geometry cannot be extracted the way every other
	 *         terrain can: its collision is a Chaos heightfield reached through no UBodySetup, so there is nothing for
	 *         FNRawMeshFactory to read and it skips landscape primitives outright. Callers that need the surface have
	 *         to sample it instead.
	 */
	static bool IsLandscapeActor(const AActor* Actor);

	/**
	 * Identify an actor whose terrain is a Mesh Terrain section.
	 * @param Actor Candidate actor under inspection. A null actor returns false.
	 * @return true when the actor is a built terrain section, or owns a Mesh Terrain collision primitive.
	 * @remark The counterpart to IsLandscapeActor, and the half that needs the transient exemption: Mesh Partition
	 *         spawns its editor representation as transient actors, where a landscape is an ordinary saved actor.
	 */
	static bool IsMeshTerrainActor(const AActor* Actor);

	/**
	 * Test whether a primitive is reporting real geometry rather than the engine's placeholder bounds.
	 * @param Primitive Primitive component to inspect.
	 * @return false when the component's bounds are the near-zero box the engine substitutes for empty geometry.
	 * @note Mesh Partition components return a deliberately tiny box — not an invalid one — while a section has no
	 *       geometry to describe, either because its build has not finished or because it covers nothing. Both
	 *       UMeshPartitionCollisionComponent::CalcBounds and UPreviewMeshComponent::CalcBounds do this, each
	 *       explaining that an empty box would spam other engine systems.
	 */
	static bool HasBuiltGeometry(const UPrimitiveComponent* Primitive);

	/**
	 * Union of an actor's registered primitive bounds, skipping any primitive still reporting placeholder bounds.
	 * @param Actor Actor whose primitives should be measured. A null actor returns an invalid box.
	 * @param bIncludeNonColliding When true, primitives with collision disabled also contribute.
	 * @return The combined bounds, or an invalid box when nothing qualified.
	 * @remark Differs from AActor::GetComponentsBoundingBox only in dropping placeholder bounds, which that treats
	 *         as a valid point and folds in — pulling the result out to wherever the empty component happens to sit.
	 */
	static FBox GetBuiltComponentsBoundingBox(const AActor* Actor, bool bIncludeNonColliding);
};