// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "UObject/ObjectKey.h"

class AActor;
class FObjectPostSaveContext;
class FObjectPreSaveContext;
class ULevel;
class UWorld;

/**
 * Identifiers backing the per-actor selection lock.
 */
namespace NEXUS::ToolingEditor::SelectionLock
{
	/** Package meta-data key written against a locked actor; editor-only, and stripped at cook. */
	inline FName MetaKey = TEXT("NexusSelectionLocked");

	/** Value stored under MetaKey. Presence of the key is what is read; the value is there to be legible in a diff. */
	inline const TCHAR* MetaValue = TEXT("1");

	/** Scene Outliner column identifier. */
	inline FName ColumnIdentifier = TEXT("Selection Lock");
}

/**
 * Editor-only per-actor lock that keeps an actor from being picked by clicking it in a level viewport.
 *
 * A locked actor has bSelectable cleared on its primitive components, which drops them out of the
 * hit-proxy pass, so a click lands on whatever sits behind them instead. Selecting the actor from the
 * Outliner still works, and so does marquee selection, which is geometry-based and never consults hit
 * proxies. The lock persists as package meta-data on the actor and is read back when the map opens.
 *
 * The meta-data is the record of the lock; bSelectable is only ever the live application of it, and is
 * held off the components across a save so the cleared flag never reaches the package.
 * @remark Locked actors draw no selection outline and do not highlight on hover; both fall out of the
 *         same bSelectable flag and cannot be kept without giving up the click-through behavior.
 * @remark Nanite meshes cannot be locked against clicking. Nanite renders hit proxies through its own
 *         pass, which reads a per-material hit-proxy table rather than the mesh batches bSelectable
 *         gates, so a locked Nanite mesh still resolves a hit proxy and still selects.
 * @see <a href="https://nexus-framework.com/docs/tooling/editor-types/selection-lock/selection-lock-state/">FNSelectionLock</a>
 */
class NEXUSTOOLINGEDITOR_API FNSelectionLock
{
public:
	/** Bind the map, actor, save and undo delegates, then seed the cache from the open editor world. */
	static void Initialize();

	/** Unbind the lifecycle delegates and drop the cache. */
	static void Shutdown();

	/**
	 * Can this actor hold a selection lock?
	 * @param Actor The actor to test.
	 * @return true when the actor has at least one primitive component for the lock to act on.
	 */
	static bool CanLock(const AActor* Actor);

	/**
	 * Is this actor currently selection-locked?
	 * @param Actor The actor to test.
	 * @return true when the actor is locked.
	 */
	static bool IsLocked(const AActor* Actor);

	/**
	 * Lock or unlock a single actor, writing the change through to package meta-data.
	 * @param Actor The actor to change.
	 * @param bLocked The lock state to apply.
	 */
	static void SetLocked(AActor* Actor, bool bLocked);

	/**
	 * Flip a group of actors as one. The group locks unless every lockable actor in it is already
	 * locked, so a mixed selection resolves to locked rather than splitting.
	 * @param Actors The actors to toggle.
	 */
	static void ToggleLocked(const TArray<AActor*>& Actors);

	/**
	 * Clear every selection lock in a world.
	 * @param World The world to sweep; nothing happens when null.
	 * @return The number of actors that were unlocked.
	 */
	static int32 UnlockAll(UWorld* World);

private:
	/** Locked actors in the open editor world, standing in for a meta-data lookup on the read path. */
	static TSet<FObjectKey> LockedActors;

	/** Handle for the map-opened delegate that rebuilds the cache. */
	static FDelegateHandle MapOpenedHandle;

	/** Handle for the actor-added delegate that catches spawned, pasted and streamed-in actors. */
	static FDelegateHandle LevelActorAddedHandle;

	/** Handle for the level-added delegate that catches sublevels and level instances. */
	static FDelegateHandle LevelAddedToWorldHandle;

	/** Handle for the objects-replaced delegate that re-applies the lock after reinstancing and construction-script reruns. */
	static FDelegateHandle ObjectsReplacedHandle;

	/** Handle for the pre-save delegate that lifts the lock off the components for the duration of a save. */
	static FDelegateHandle PreSaveWorldHandle;

	/** Handle for the post-save delegate that puts it back. */
	static FDelegateHandle PostSaveWorldHandle;

	/** Handle for the undo delegate that re-asserts the lock after a transaction is rolled back or replayed. */
	static FDelegateHandle PostUndoRedoHandle;

	/**
	 * Push the lock state onto the actor's primitive components, dirtying the render state of any that move.
	 * @param Actor The actor to change.
	 * @param bLocked The lock state to apply.
	 * @return true when at least one component changed, meaning cached hit proxies are now stale.
	 */
	static bool ApplyLock(AActor* Actor, bool bLocked);

	/**
	 * Collect the cache as live actors, pruning keys whose actor has since gone.
	 * @param World Restrict the result to actors in this world; pass null to accept any.
	 * @return The resolved actors.
	 */
	static TArray<AActor*> GetLockedActors(const UWorld* World);

	/** @return true when the actor carries the lock key in its package meta-data. */
	static bool ReadMetaData(const AActor* Actor);

	/** Add or remove the lock key in the actor's package meta-data, dirtying the package. */
	static void WriteMetaData(const AActor* Actor, bool bLocked);

	/**
	 * Bring one actor's components in line with its meta-data, updating its cache entry either way.
	 * @param Actor The actor to reconcile.
	 * @return true when at least one component changed.
	 */
	static bool RestoreActor(AActor* Actor);

	/** Reconcile every actor in a world, invalidating hit proxies once at the end if anything moved. */
	static void RestoreWorld(UWorld* World);

	/** Reconcile every actor in a level, invalidating hit proxies once at the end if anything moved. */
	static void RestoreLevel(ULevel* Level);

	/** Drop the cache and rebuild it from scratch against a world. */
	static void RebuildForWorld(UWorld* World);

	/** Set or lift bSelectable on the world's locked actors without touching render state. */
	static void SetLocksApplied(UWorld* World, bool bApplied);

	/** Map-opened handler; discards the previous world's cache and rebuilds it. */
	static void OnMapOpened(const FString& Filename, bool bAsTemplate);

	/** Actor-added handler; restores the lock on an actor that arrives already carrying the key. */
	static void OnLevelActorAdded(AActor* Actor);

	/** Level-added handler; restores locks across a sublevel or level instance as it comes in. */
	static void OnLevelAddedToWorld(ULevel* Level, UWorld* World);

	/** Objects-replaced handler; re-applies the lock after actor reinstancing and construction-script reruns. */
	static void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);

	/** Pre-save handler; lifts the lock off the components so the cleared flag is not serialized. */
	static void OnPreSaveWorld(UWorld* World, FObjectPreSaveContext SaveContext);

	/** Post-save handler; puts the lock back on the components. */
	static void OnPostSaveWorld(UWorld* World, FObjectPostSaveContext SaveContext);

	/** Undo handler; re-asserts the lock, which a rolled-back transaction can otherwise strip. */
	static void OnPostUndoRedo();
};
