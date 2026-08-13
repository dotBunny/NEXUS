// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "UObject/ObjectKey.h"

class AActor;
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
 * @remark Locked actors draw no selection outline and do not highlight on hover; both fall out of the
 *         same bSelectable flag and cannot be kept without giving up the click-through behavior.
 * @see <a href="https://nexus-framework.com/docs/plugins/tooling/editor-types/selection-lock/selection-lock/">FNSelectionLock</a>
 */
class NEXUSTOOLINGEDITOR_API FNSelectionLock
{
public:
	/** Bind the map and actor lifecycle delegates, then seed the cache from the open editor world. */
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

	/** Handle for the objects-replaced delegate that re-keys the cache after Blueprint reinstancing. */
	static FDelegateHandle ObjectsReplacedHandle;

	/** Push the lock state onto the actor's primitive components and dirty their render state. */
	static void ApplyLock(AActor* Actor, bool bLocked);

	/** @return true when the actor carries the lock key in its package meta-data. */
	static bool ReadMetaData(const AActor* Actor);

	/** Add or remove the lock key in the actor's package meta-data, dirtying the package. */
	static void WriteMetaData(const AActor* Actor, bool bLocked);

	/** Rebuild the cache from meta-data across a whole world, applying the lock as it goes. */
	static void CacheWorld(UWorld* World);

	/** Map-opened handler; discards the previous world's cache and rebuilds it. */
	static void OnMapOpened(const FString& Filename, bool bAsTemplate);

	/** Actor-added handler; restores the lock on an actor that arrives already carrying the key. */
	static void OnLevelActorAdded(AActor* Actor);

	/** Objects-replaced handler; moves cache entries onto reinstanced actors and re-applies the lock. */
	static void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);
};
