// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class FObjectPreSaveContext;
class UObject;
class UWorld;
struct FPropertyChangedEvent;

/**
 * Keeps each level's baked world-collision cache current by re-baking what has changed when the level is saved.
 *
 * Two layers decide whether a save does any work, and they answer different questions:
 *
 * - A per-world **dirty flag**, driven by the same editor change delegates the collision visualizer watches, answers
 *   "has any collision geometry been touched since the last bake?". A clean world skips the save-time pass entirely.
 * - The stored **fingerprint**, recomputed per organ inside the bake, answers "is this organ's cache still correct?".
 *   This is the authority, and it is what the runtime checks too.
 *
 * The flag exists only to keep an untouched level's save free; correctness never rests on it. A world this session
 * has not seen an edit for is treated as dirty rather than clean, so a change made before these hooks were live — by
 * a commandlet, a script, or an earlier session — costs one fingerprint pass rather than being missed.
 * @note Editor-only, and bound for the editor session. Never runs during a cook save.
 * @see FNWorldCollisionBaker
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldCollisionCacheSave
{
public:
	/** Subscribe to the world-save and geometry-change delegates. Call once from the module's startup. */
	static void Register();

	/** Drop every subscription. Call from the module's ShutdownModule. */
	static void Unregister();

	/**
	 * World pre-save hook: bake whatever has changed into the level's collision cache.
	 * @param World World being saved.
	 * @param ObjectPreSaveContext Save context; a cook save returns immediately.
	 * @note Everything the bake writes — the organ components and the cache actor — lives in the level being saved,
	 *       so unlike the cell side-car there is no second package to flush afterwards.
	 */
	static void OnPreSaveWorld(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext);

	/** Mark World as needing a bake on its next save. */
	static void MarkDirty(const UWorld* World);

	/** Mark World as baked, so its next save can skip the fingerprint pass unless something changes first. */
	static void MarkClean(const UWorld* World);

	/**
	 * @return true when World should be re-baked on save.
	 * @param World World to test; a world never seen before reports dirty, which is the safe answer.
	 */
	static bool IsDirty(const UWorld* World);

private:
	/** @return true when a change to Actor could alter what a collision gather would emit. */
	static bool IsRelevantActor(const AActor* Actor);

	/** Resolve a UObject reported by an editor change delegate to the AActor it belongs to (or null). */
	static AActor* ResolveActor(UObject* Object);

	/**
	 * Mark the world owning Actor dirty, when Actor is collision-relevant.
	 * @note Takes a mutable pointer only to match the level-actor delegates' signature; nothing here mutates it.
	 */
	static void MarkDirtyForActor(AActor* Actor);

	/** Property-change handler; ignores the continuous mid-edit stream and acts on the finalizing change. */
	static void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	/** Worlds known to have been baked with no collision edit since; anything absent is treated as dirty. */
	static TSet<TWeakObjectPtr<const UWorld>> CleanWorlds;

	static FDelegateHandle PreSaveWorldHandle;
	static FDelegateHandle ActorAddedHandle;
	static FDelegateHandle ActorDeletedHandle;
	static FDelegateHandle EndObjectMovementHandle;
	static FDelegateHandle PropertyChangedHandle;
	static FDelegateHandle PostUndoRedoHandle;
};
