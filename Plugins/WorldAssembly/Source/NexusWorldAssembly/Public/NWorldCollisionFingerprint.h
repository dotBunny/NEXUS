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
	 * @return The fingerprint, or 0 when World is null.
	 */
	static uint64 Compute(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
		const FNWorldAssemblyWorldCollisionSettings& Settings);

	/**
	 * Fingerprint an already-filtered actor set, for callers that have just gathered one and would rather not walk
	 * the world a second time.
	 * @param Actors Actors that passed the world-actor filter. Not re-filtered here.
	 * @param Bounds Containment bounds, applied to Actors exactly as the raw-mesh factory applies them.
	 * @param Settings Collision-capture settings; hashed in, not re-applied.
	 * @return The fingerprint.
	 */
	static uint64 ComputeForActors(const TArray<AActor*>& Actors, const TArray<FBoxSphereBounds>& Bounds,
		const FNWorldAssemblyWorldCollisionSettings& Settings);

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
	 * @return One actor's contribution: its identity, plus the placement and collision geometry of every primitive
	 * on it that the raw-mesh factory would read. Returns 0 for a null or invalid actor.
	 * @param Actor Actor to summarize.
	 * @note Instanced primitives contribute every instance transform, because moving one instance changes the
	 *       emitted geometry exactly as moving a whole actor does.
	 */
	static uint64 HashActor(const AActor* Actor);

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
