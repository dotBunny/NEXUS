// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Types/NRawMesh.h"

class AActor;
class UWorld;

/**
 * Editor-only shared cache of a level's merged world-collision mesh — the same geometry definition World Assembly
 * uses (the simple collision of every actor passing the world-actor filter, merged into one mesh).
 *
 * Built lazily per UWorld and invalidated when world geometry changes (actor added/removed/moved, finalized property
 * edits, undo/redo), so author-time diagnostics such as bone penetration can sample it cheaply instead of each one
 * re-gathering the whole level on every selection. The cache is a process-lifetime static; its invalidation hooks are
 * registered once on first use and live for the editor session.
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldCollisionCache
{
public:
	/** @return The merged world-collision mesh for World, rebuilt if the world changed or the cache was invalidated. */
	static const FNRawMesh& Get(const UWorld* World);

	/**
	 * Stateless gather: merge the simple collision of every actor passing the World Assembly world-actor filter that
	 * overlaps Bounds into one world-space mesh. This is the shared "what counts as world geometry" definition; Get
	 * memoizes it. Exposed (and unit-tested) directly so the build can be exercised without the cache's static state.
	 * @param World World to gather from; returns an empty mesh when null.
	 * @param Bounds Containment bounds; an actor contributes when its bounds overlap any one (empty = whole level).
	 * @param OutSourceActors Optional; receives the filtered world actors the mesh was gathered from.
	 * @note Game-thread only (walks live actors and flushes async static-mesh compilation).
	 */
	static FNRawMesh Build(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds, TArray<AActor*>* OutSourceActors = nullptr);

	/** Mark the cache stale so the next Get rebuilds it. */
	static void Invalidate();

private:
	/** Subscribe (once) to the editor change delegates that should invalidate the cache. */
	static void EnsureInvalidationHooks();

	static FNRawMesh CachedMesh;
	static TWeakObjectPtr<const UWorld> CachedWorld;
	static bool bDirty;
	static bool bHooksRegistered;
};
