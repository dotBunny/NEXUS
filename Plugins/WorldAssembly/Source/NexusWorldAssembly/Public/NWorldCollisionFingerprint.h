// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UPrimitiveComponent;
class UWorld;
struct FNWorldAssemblyWorldCollisionSettings;

/**
 * Decides whether a baked world-collision cache still describes the world.
 *
 * A fingerprint is a 64-bit summary of everything a gather would read: the region asked for, the settings that
 * govern which actors qualify, and the identity, placement, and collision geometry of every actor that passes. Two
 * gathers that would produce the same meshes produce the same fingerprint; any change that would produce different
 * meshes changes it.
 *
 * The summary is **order-independent** — per-actor contributions are folded together commutatively — because the
 * order FNActorUtils::GetWorldActors walks a level in is not stable between the editor that bakes a cache and the
 * game that reads it. A fingerprint that depended on iteration order would mismatch on every run and quietly turn
 * the whole cache off.
 * @note Game-thread only: walks live actors and their registered components.
 * @note Cheap relative to the gather it guards — it reads transforms and body-setup identity rather than extracting,
 *       hull-converting, and merging geometry — but it is not free, and it is still O(actors).
 * @see FNWorldCollisionSourceKey
 */
class NEXUSWORLDASSEMBLY_API FNWorldCollisionFingerprint
{
public:
	/**
	 * Fingerprint the geometry a gather over Bounds under Settings would see in World.
	 * @param World World to inspect; returns 0 when null.
	 * @param Bounds Containment bounds; an actor contributes when its bounds overlap any one (empty = whole level).
	 * @param Settings Collision-capture settings governing which actors qualify.
	 * @param OutContributors Optional; receives how many actors the fingerprint was folded from.
	 * @return The fingerprint, or 0 when World is null.
	 * @note The actor count is worth having beside the fingerprint when one fails to match, because the commonest
	 *       reason for two fingerprints of the "same" level to differ is that they covered different numbers of
	 *       actors — the walk sees only what is loaded, and in a World Partition level that is whatever regions and
	 *       data layers happen to be up. A differing count says the world was not the same world; an equal count
	 *       with a differing fingerprint says something about the geometry itself moved.
	 */
	static uint64 Compute(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
		const FNWorldAssemblyWorldCollisionSettings& Settings, int32* OutContributors = nullptr);

	/**
	 * Fingerprint an already-filtered actor set, for callers that have just gathered one and would rather not walk
	 * the world a second time.
	 * @param Actors Actors that passed the world-actor filter. Not re-filtered here.
	 * @param Bounds Containment bounds, applied to Actors exactly as the raw-mesh factory applies them.
	 * @param Settings Collision-capture settings; hashed in, not re-applied.
	 * @param OutContributors Optional; receives how many of Actors actually contributed — those that were valid and
	 *        overlapped Bounds. See Compute's note for why this is worth reporting alongside a mismatch.
	 * @return The fingerprint.
	 */
	static uint64 ComputeForActors(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& Bounds,
		const FNWorldAssemblyWorldCollisionSettings& Settings, int32* OutContributors = nullptr);

	/**
	 * Fingerprint the landscape state a sampling pass would read, for the level-scoped landscape section.
	 * @param World World to inspect; returns 0 when null.
	 * @param Settings Collision-capture settings; the sample spacing and the capture toggle both reach the hash.
	 * @return The fingerprint, or 0 when World is null or landscape capture is off.
	 * @remark Coarser than the actor fingerprint, and knowingly so. A landscape's height data is not reachable as
	 *         saved state a packaged build can hash, so this covers the landscape actors' identity, placement, and
	 *         collision-component extents rather than the heights themselves. Sculpting a landscape without changing
	 *         its extents is therefore invisible *here* — it is caught at author time by the edit-driven dirty flag,
	 *         which is the only context in which a landscape can be sculpted at all.
	 */
	static uint64 ComputeLandscape(const UWorld* World, const FNWorldAssemblyWorldCollisionSettings& Settings);

	/**
	 * One actor's or one world's contribution, stopped short at each ingredient it is built from.
	 *
	 * Every field is a cumulative prefix of the next: Identity covers actor and component names, Bodies adds the
	 * cooked-collision GUID and the collision flags, Tags adds the component tags, and Transforms adds the placements
	 * and is the fingerprint proper. Comparing two of these tells you *which* ingredient moved when a cache is
	 * rejected, which the fingerprint alone cannot — it is one number, and any change scrambles all of it.
	 */
	struct FLayers
	{
		/** Actor and component identity alone. Stable across sessions unless actors are added, removed or renamed. */
		uint64 Identity = 0;
		/** Identity plus each body's cooked-collision GUID, trace flag and collision setting. */
		uint64 Bodies = 0;
		/** The above plus, per component, whether the gather's ignore list excludes it. */
		uint64 Tags = 0;
		/**
		 * The above plus every transform, folded without regard to order.
		 * @note Not a prefix of Transforms but a sibling of it: the same placement data read two ways. The pair is
		 *       what separates instances being reordered from instances actually moving — a component that rebuilds
		 *       its instance list on load, as a hierarchical mesh or a generator's output does, presents the same
		 *       transforms at different indices.
		 */
		uint64 Placement = 0;
		/** Identity, bodies, tags plus every transform in index order — the same fold HashActor returns. */
		uint64 Transforms = 0;
		/** How many primitives contributed. Reported so a changed hash can be read against a changed shape. */
		int32 Primitives = 0;
		/** How many instances contributed across those primitives; zero for an actor holding no instanced mesh. */
		int32 Instances = 0;
	};

	/**
	 * Log the layer breakdown of what a fingerprint of World would cover, at Warning.
	 *
	 * @param World World to inspect; null does nothing.
	 * @param Bounds Containment bounds, applied exactly as Compute applies them.
	 * @param Settings Collision-capture settings governing which actors qualify.
	 * @remark For diagnosing a cache that will not validate. Capture it from two sessions and compare: the first
	 *         layer whose value differs names the ingredient that moved. Identity differing means the walk saw a
	 *         different set of actors; Bodies means a body setup was rebuilt rather than loaded (a body built at
	 *         runtime gets a fresh GUID every session); Tags means component tags are being rewritten; Transforms
	 *         means something genuinely moved.
	 * @note Walks the world a second time, so it is called only on the rejection path, which is about to pay for a
	 *       full fresh gather anyway.
	 */
	static void LogLayers(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
		const FNWorldAssemblyWorldCollisionSettings& Settings);

	/**
	 * @return One actor's contribution: its identity, plus the placement and collision geometry of every primitive
	 * on it that the raw-mesh factory would read. Returns 0 for a null or invalid actor.
	 * @param Actor Actor to summarize.
	 * @param ComponentIgnoreTags The gather's component ignore list, from
	 *        FNCreateVirtualWorldTask::CreateWorldComponentIgnoreTags. Each primitive contributes whether it is
	 *        excluded by this list, not what it is tagged with.
	 * @param OutLayers Optional; receives the same fold stopped short at each ingredient. See FLayers.
	 * @note Summarizes the *decision* a gather would make about each component rather than the tags behind it.
	 *       Hashing the tags themselves made the fingerprint sensitive to tag churn from systems that rewrite
	 *       component tags for their own bookkeeping — PCG stamps its spawned components on every generate — so a
	 *       level holding generated content hashed differently every session and its cache could never validate.
	 * @note Instanced primitives contribute every instance transform, because moving one instance changes the
	 *       emitted geometry exactly as moving a whole actor does.
	 * @note Summarizes a superset of what a gather emits, rather than reproducing its exclusions. A primitive the
	 *       gather would skip still contributes the state that got it skipped — its collision setting and its tags —
	 *       so changing either shows up here without this having to know the ignore lists at all.
	 */
	static uint64 HashActor(const AActor* Actor, const TArray<FName>& ComponentIgnoreTags,
		FLayers* OutLayers = nullptr);

	/**
	 * @return A hash of the settings fields that change what a gather emits.
	 * @param Settings Settings to summarize.
	 * @remark Deliberately partial. Only the fields that govern *what geometry is gathered* belong here; a setting
	 *         that governs whether caching happens at all must never reach this hash, or toggling it would invalidate
	 *         every cache in the project.
	 */
	static uint64 HashSettings(const FNWorldAssemblyWorldCollisionSettings& Settings);

	/**
	 * @return true when Actor's bounds overlap at least one entry of Bounds, or when Bounds is empty.
	 * @param Actor Actor to test.
	 * @param Bounds Containment bounds.
	 * @note Mirrors the overlap test inside FNRawMeshFactory::FromActorsInBounds exactly — including its use of
	 *       colliding-components-only bounds that take in child actors — so the fingerprint covers precisely the
	 *       actors the gather would have read, and no others.
	 */
	static bool OverlapsBounds(const AActor* Actor, const TArray<FBoxSphereBounds>& Bounds);

	/** @return A hash of the containment bounds themselves, so that resizing an organ invalidates its cache. */
	static uint64 HashBounds(const TArray<FBoxSphereBounds>& Bounds);
};
