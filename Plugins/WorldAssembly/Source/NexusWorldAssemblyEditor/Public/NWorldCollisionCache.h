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
 *   unit tests where blocking is fine.
 * - The **asynchronous** RequestAsyncRefresh / GetPublished* path never blocks the caller: the game-thread-only gather
 *   runs on a debounced ticker, the merge + BVH build run on a low-priority background task, and results publish when
 *   ready. A superseding change cancels the in-flight build. This is the path the bone visualizer uses so a viewport
 *   redraw never stalls on a full world re-gather.
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
	 * @return A monotonic counter that changes whenever the cached mesh could differ from a previous read — i.e.
	 * on every Invalidate (actor add/remove/move, finalized property edits, undo/redo) and on world switch.
	 * This is the "did the world change?" signal consumers memoize against; it never regresses within a session.
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

	/** Mark the cache stale so the next Get rebuilds it (and schedules an async rebuild for the published data). */
	static void Invalidate();

	/**
	 * Non-blocking: ensure an up-to-date merged mesh + BVH is being (re)built for World in the background, and return
	 * immediately. Safe (and cheap) to call every frame — it only kicks work when the cache is stale or the world
	 * changed. Read the results via GetPublishedBVH / GetPublishedMesh, re-reading whenever GetResultsGeneration changes.
	 * @param World World whose collision to track; ignored when null.
	 */
	static void RequestAsyncRefresh(const UWorld* World);

	/** @return The most recently published BVH, or null when no background build has finished yet. */
	static TSharedPtr<const FNMeshBVH> GetPublishedBVH();

	/** @return The most recently published merged mesh, or null when no background build has finished yet. */
	static TSharedPtr<const FNRawMesh> GetPublishedMesh();

	/** @return true when the published mesh is convex (drives the visualizer's exact-path fallback for trivial levels). */
	static bool IsPublishedMeshConvex();

	/** @return true when the published mesh contains non-triangle loops (also routes to the exact fallback path). */
	static bool PublishedMeshHasNonTris();

	/** @return A counter bumped each time new background results publish; consumers memoize against it. */
	static uint32 GetResultsGeneration();

	/** Test hook: drive any pending async refresh for World to completion synchronously (gather + build + publish). */
	static void FlushAsyncRefreshForTesting(const UWorld* World);

	/** Remove the async ticker and wait for any in-flight build. Call from the owning module's ShutdownModule. */
	static void Shutdown();

private:
	/** Subscribe (once) to the editor change delegates that should invalidate the cache. */
	static void EnsureInvalidationHooks();

	/** Register (once) the game-thread ticker that drives the async rebuild state machine. */
	static void EnsureAsyncPump();

	/** One iteration of the async state machine: publish finished work and/or start a new debounced gather. */
	static void PumpOnce();

	/** Ticker entry point; runs PumpOnce and keeps ticking. */
	static bool Pump(float DeltaTime);

	/** Result produced by a background build and swapped in atomically on the game thread at publish time. */
	struct FRebuildResult
	{
		TSharedPtr<const FNRawMesh> Mesh;
		TSharedPtr<const FNMeshBVH> BVH;
		TArray<FObjectKey> SourceActors;
		bool bMeshIsConvex = false;
		bool bMeshHasNonTris = false;
	};

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
	 * geometry we last gathered (covers delete / collision-off / ignore-tag transitions) or it passes the world-actor
	 * filter now (covers add / collision-on). Mirrors FNWorldAssemblyEdMode::ShouldRebuildForActor so the cache and the
	 * ed mode's collision visualizer invalidate on exactly the same events — notably NOT on moving a bone, which is a
	 * generation input rather than collision geometry.
	 */
	static bool IsRelevantActor(const AActor* Actor);

	// --- Synchronous path (Get / GetBVH / Build; used by direct callers and unit tests) ---
	static FNRawMesh CachedMesh;
	static FNMeshBVH CachedBVH;
	static TWeakObjectPtr<const UWorld> CachedWorld;
	/** Object keys of the actors the last gather processed; lets the hooks detect changes to a now-removed source. */
	static TSet<FObjectKey> CachedSourceActors;
	static bool bDirty;
	static bool bBVHDirty;
	static bool bHooksRegistered;
	static uint32 Generation;

	// --- Asynchronous path (RequestAsyncRefresh / GetPublished*; used by the bone visualizer) ---
	/** World the async pipeline is tracking; a change resets the published data and rebuilds from scratch. */
	static TWeakObjectPtr<const UWorld> AsyncWorld;
	/** Latest published merged mesh + BVH. Written on the game thread at publish, read on the game thread by consumers. */
	static TSharedPtr<const FNRawMesh> PublishedMesh;
	static TSharedPtr<const FNMeshBVH> PublishedBVH;
	static bool bPublishedMeshIsConvex;
	static bool bPublishedMeshHasNonTris;
	/** Bumped on every publish (and on world switch) so consumers know to refresh their memo. */
	static uint32 ResultsGeneration;
	/** Set when a relevant change lands; the pump consumes it to start the next gather. */
	static bool bAsyncDirty;
	/** True between starting a gather and consuming its background result. */
	static bool bRebuildInFlight;
	/** Monotonic id bumped on every invalidation; a finished build whose id is stale is discarded (superseded). */
	static uint64 RebuildRequestId;
	/** The RebuildRequestId the in-flight build was started for. */
	static uint64 InFlightRequestId;
	/** Handle to the background merge + BVH task; polled for completion on the ticker. */
	static UE::Tasks::TTask<FRebuildResult> InFlightTask;
	/** Cooperative-cancel flag for the in-flight task; set true when a newer change supersedes it. */
	static TSharedPtr<FThreadSafeBool, ESPMode::ThreadSafe> InFlightCancelFlag;
	/** FPlatformTime seconds of the last invalidation; the pump waits a short debounce past this before gathering. */
	static double LastInvalidateTime;
	/** FPlatformTime seconds of the last visualizer refresh request; the pump only gathers while a bone drew recently. */
	static double LastDrawTime;
	static bool bPumpRegistered;
	static FTSTicker::FDelegateHandle PumpHandle;
};
