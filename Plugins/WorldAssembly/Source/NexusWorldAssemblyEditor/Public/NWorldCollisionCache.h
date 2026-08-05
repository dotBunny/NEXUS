// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "HAL/ThreadSafeBool.h"
#include "Math/NMeshBVH.h"
#include "Tasks/Task.h"
#include "Types/NRawMesh.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class UObject;
class UWorld;

/**
 * Editor-only shared cache of a level's merged world-collision mesh — the same geometry definition World Assembly
 * uses (the simple collision of every actor passing the world-actor filter, merged into one mesh).
 *
 * Built lazily per UWorld and invalidated when world geometry changes (actor added/removed/moved, finalized property
 * edits, undo/redo), so author-time diagnostics such as bone penetration can sample it cheaply instead of each one
 * re-gathering the whole level on every selection. The cache is a process-lifetime static; its invalidation hooks are
 * registered once on first use and live for the editor session.
 *
 * Two consumption paths exist:
 * - The **synchronous** Get / GetBVH / Build rebuild in-place on the calling (game) thread. Used by direct callers and
 *   unit tests where blocking is fine. These memoize a single world at a time.
 * - The **asynchronous** RequestAsyncRefresh / GetPublished* path never blocks the caller: the game-thread-only gather
 *   runs on a debounced ticker, the merge + BVH build run on a low-priority background task, and results publish when
 *   ready. A superseding change cancels the in-flight build. This is the path the bone visualizer uses so a viewport
 *   redraw never stalls on a full world re-gather. It is tracked **per UWorld** (keyed by the world each query names):
 *   the level viewport and any Blueprint-editor preview scenes are sampled at the same time, so each world keeps its
 *   own published mesh/BVH, results generation, and rebuild pipeline and none of them clobber the others.
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldCollisionCache
{
public:
	/** @return The merged world-collision mesh for World, rebuilt if the world changed or the cache was invalidated. */
	static const FNRawMesh& Get(const UWorld* World);

	/**
	 * @return A BVH over the merged world-collision mesh for World, rebuilt in lockstep with Get's mesh.
	 * Built lazily on first request after a rebuild, so worlds whose bones are never sampled never pay for it.
	 * Accelerates the bone visualizer's per-corner penetration sampling from O(world triangles) to ~O(log).
	 */
	static const FNMeshBVH& GetBVH(const UWorld* World);

	/**
	 * @return A monotonic counter for the **synchronous** cache that changes whenever its cached mesh could differ from
	 * a previous read — i.e. on every Invalidate and on world switch. Never regresses within a session. Async consumers
	 * use the per-world GetResultsGeneration instead.
	 */
	static uint32 GetGeneration() { return Generation; }

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

	/**
	 * Mark the synchronous cache stale so the next Get rebuilds it, and mark **every** tracked world's async pipeline
	 * stale as well. Used for world-agnostic changes (undo/redo) that could touch any world; actor-scoped edits go
	 * through the private per-world path instead.
	 */
	static void Invalidate();

	/**
	 * Non-blocking: ensure an up-to-date merged mesh + BVH is being (re)built for World in the background, and return
	 * immediately. Safe (and cheap) to call every frame — it only kicks work when that world is stale or first seen.
	 * Read the results via GetPublishedBVH / GetPublishedMesh(World), re-reading whenever GetResultsGeneration(World)
	 * changes. Each world is tracked independently, so refreshing one never disturbs another's published data.
	 * @param World World whose collision to track; ignored when null.
	 */
	static void RequestAsyncRefresh(const UWorld* World);

	/** @return World's most recently published BVH, or null when no background build has finished for it yet. */
	static TSharedPtr<const FNMeshBVH> GetPublishedBVH(const UWorld* World);

	/** @return World's most recently published merged mesh, or null when no background build has finished for it yet. */
	static TSharedPtr<const FNRawMesh> GetPublishedMesh(const UWorld* World);

	/** @return true when World's published mesh is convex (drives the visualizer's exact-path fallback). */
	static bool IsPublishedMeshConvex(const UWorld* World);

	/** @return true when World's published mesh contains non-triangle loops (also routes to the exact fallback path). */
	static bool PublishedMeshHasNonTris(const UWorld* World);

	/** @return A per-world counter bumped each time new background results publish for World; consumers memoize against it. */
	static uint32 GetResultsGeneration(const UWorld* World);

	/** Test hook: drive any pending async refresh for World to completion synchronously (gather + build + publish). */
	static void FlushAsyncRefreshForTesting(const UWorld* World);

	/** Remove the async ticker and wait for every in-flight build. Call from the owning module's ShutdownModule. */
	static void Shutdown();

private:
	/** Subscribe (once) to the editor change delegates that should invalidate the cache. */
	static void EnsureInvalidationHooks();

	/** Register (once) the game-thread ticker that drives the async rebuild state machine for every tracked world. */
	static void EnsureAsyncPump();

	/** Result produced by a background build and swapped in atomically on the game thread at publish time. */
	struct FRebuildResult
	{
		TSharedPtr<const FNRawMesh> Mesh;
		TSharedPtr<const FNMeshBVH> BVH;
		TArray<FObjectKey> SourceActors;
		bool bMeshIsConvex = false;
		bool bMeshHasNonTris = false;
	};

	/**
	 * The full async pipeline for one UWorld: its last published results plus the in-flight rebuild bookkeeping.
	 * One entry lives in AsyncStates per world ever sampled via RequestAsyncRefresh; all fields are game-thread only
	 * except the cancel flag (shared with the background task) and the published Shared pointers (written on the game
	 * thread at publish, read on the game thread by consumers).
	 */
	struct FAsyncWorldState
	{
		/** Latest published merged mesh + BVH for this world. */
		TSharedPtr<const FNRawMesh> PublishedMesh;
		TSharedPtr<const FNMeshBVH> PublishedBVH;
		bool bPublishedMeshIsConvex = false;
		bool bPublishedMeshHasNonTris = false;
		/** Bumped on every publish so this world's consumers know to refresh their memo. */
		uint32 ResultsGeneration = 0;
		/** Object keys the last published build gathered; lets the hooks scope invalidation to this world's geometry. */
		TSet<FObjectKey> SourceActors;

		/** Set when a relevant change lands for this world; the pump consumes it to start the next gather. */
		bool bAsyncDirty = false;
		/** True between starting this world's gather and consuming its background result. */
		bool bRebuildInFlight = false;
		/** Monotonic id bumped on every invalidation of this world; a finished build with a stale id is discarded. */
		uint64 RebuildRequestId = 0;
		/** The RebuildRequestId this world's in-flight build was started for. */
		uint64 InFlightRequestId = 0;
		/** Handle to this world's background merge + BVH task; polled for completion on the ticker. */
		UE::Tasks::TTask<FRebuildResult> InFlightTask;
		/** Cooperative-cancel flag for the in-flight task; set true when a newer change supersedes it. */
		TSharedPtr<FThreadSafeBool, ESPMode::ThreadSafe> InFlightCancelFlag;
		/** FPlatformTime seconds of the last invalidation; the pump waits a short debounce past this before gathering. */
		double LastInvalidateTime = 0.0;
		/** FPlatformTime seconds of the last visualizer refresh; the pump only gathers while a bone drew recently. */
		double LastDrawTime = 0.0;
	};

	/** Mark one world's async pipeline stale, bump its request id, and ask any in-flight build for it to bail early. */
	static void MarkStateDirty(FAsyncWorldState& State);

	/** Invalidate a single world — the synchronous cache too when it matches. The actor-scoped invalidation path. */
	static void InvalidateWorld(const UWorld* World);

	/** One iteration of the async state machine across every tracked world: publish finished work and/or start gathers. */
	static void PumpOnce();

	/** Advance one world's async state machine (publish then, if eligible, gather + launch). Re-finds around the gather. */
	static void PumpState(const UWorld* World, const TWeakObjectPtr<const UWorld>& Key, double Now);

	/** Ticker entry point; runs PumpOnce and keeps ticking. */
	static bool Pump(float DeltaTime);

	/** Game-thread gather: the filtered world actors' simple collision as raw (unmerged) meshes + world transforms. */
	static void GatherRaw(const UWorld* World, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms,
		TArray<AActor*>& OutSourceActors);

	/** Thread-safe: merge the gathered meshes, validate, and build the BVH into a publishable result. Honors Cancel. */
	static FRebuildResult MergeAndBuild(TArray<FNRawMesh> Meshes, TArray<FTransform> Transforms,
		TArray<FObjectKey> SourceActors, TSharedRef<FThreadSafeBool, ESPMode::ThreadSafe> Cancel);

	/** Resolve a UObject reported by an editor change delegate to the AActor it belongs to (or null). */
	static AActor* ResolveActor(UObject* Object);

	/**
	 * @return true when a change to Actor could alter the merged world-collision mesh — i.e. it was part of the
	 * geometry we last gathered for its world (covers delete / collision-off / ignore-tag transitions) or it passes
	 * the world-actor filter now (covers add / collision-on). Mirrors UNWorldAssemblyEdMode::ShouldRebuildForActor so
	 * the cache and the ed mode's collision visualizer invalidate on exactly the same events — notably NOT on moving a
	 * bone, which is a generation input rather than collision geometry.
	 */
	static bool IsRelevantActor(const AActor* Actor);

	// --- Synchronous path (Get / GetBVH / Build; used by direct callers and unit tests) ---
	static FNRawMesh CachedMesh;
	static FNMeshBVH CachedBVH;
	static TWeakObjectPtr<const UWorld> CachedWorld;
	/** Object keys of the actors the last synchronous gather processed; lets the hooks detect changes to a removed source. */
	static TSet<FObjectKey> CachedSourceActors;
	static bool bDirty;
	static bool bBVHDirty;
	static bool bHooksRegistered;
	static uint32 Generation;

	// --- Asynchronous path (RequestAsyncRefresh / GetPublished*; used by the bone visualizer), one entry per world ---
	static TMap<TWeakObjectPtr<const UWorld>, FAsyncWorldState> AsyncStates;
	static bool bPumpRegistered;
	static FTSTicker::FDelegateHandle PumpHandle;
};
