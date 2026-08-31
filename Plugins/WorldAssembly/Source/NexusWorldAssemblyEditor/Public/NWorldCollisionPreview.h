// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Math/NMeshBVH.h"
#include "Types/NRawMesh.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class UObject;
class UWorld;
struct FPropertyChangedEvent;

/**
 * Editor-side view of a level's world collision, derived solely from its baked pool.
 *
 * There is one source of world collision in the plugin — the pool an ANWorldCollisionCacheActor holds — and this is
 * how the editor reads it. The collision visualizer draws the mesh; the bone penetration readout samples the BVH.
 * Both therefore show exactly the geometry an assembly will collide against, because it is the same geometry, not a
 * reconstruction of it.
 *
 * This replaced a cache that gathered the live world on its own, behind an async pipeline built to keep a constantly
 * changing source off the draw thread. A baked pool does not change while you work — it changes when you bake — so
 * none of that machinery is needed: the merge and BVH are memoized per world and rebuilt only when the pool or the
 * world's geometry changes.
 * @note An unbaked level has nothing to show. That is reported through GetState rather than papered over by gathering,
 *       so what is on screen is never a different answer from what an assembly would get.
 * @see FNWorldCollisionBaker
 * @see FNWorldCollisionPool
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldCollisionPreview
{
public:
	/** Why a world's collision preview is or is not available. */
	enum class EState : uint8
	{
		/** A current pool answered for every organ; the mesh and BVH are usable. */
		Available,
		/** The level has never been baked, or holds no organs to bake. */
		NotBaked,
		/** A pool exists but no longer matches the world; it needs re-baking before it can be shown. */
		Stale
	};

	/**
	 * @return World's merged, baked world-collision mesh, or an empty mesh when GetState is not Available.
	 * @param World World to read; null yields the empty mesh.
	 */
	static const FNRawMesh& GetMesh(const UWorld* World);

	/**
	 * @return A BVH over GetMesh, built lazily on first request after a rebuild.
	 * @param World World to read; null yields an empty BVH.
	 */
	static const FNMeshBVH& GetBVH(const UWorld* World);

	/**
	 * @return Whether World's collision can be shown, and if not, why.
	 * @param World World to inspect.
	 * @note Validating costs one pass over the filtered actors, so the answer is memoized and recomputed only when a
	 *       geometry edit or a bake invalidates it — not on every query, which is what lets a viewport redraw call it.
	 */
	static EState GetState(const UWorld* World);

	/** @return true when GetState is Available. */
	static bool IsAvailable(const UWorld* World) { return GetState(World) == EState::Available; }

	/**
	 * @return A monotonic counter bumped whenever World's published mesh could differ from a previous read.
	 * @param World World to inspect. Consumers memoize their own derived values against this.
	 */
	static uint32 GetGeneration(const UWorld* World);

	/**
	 * Drop what is memoized for World, so the next query re-validates and rebuilds.
	 * @param World World to invalidate; null invalidates every world.
	 * @remark Call after baking. The geometry-change delegates registered by Register() cover author edits.
	 */
	static void Invalidate(const UWorld* World = nullptr);

	/** Subscribe to the geometry-change delegates that invalidate the preview. Call once from module startup. */
	static void Register();

	/** Drop subscriptions and memoized state. Call from the module's ShutdownModule. */
	static void Unregister();

	/**
	 * Show a notification explaining that collision cannot be previewed, offering to bake.
	 * @param World World the notification is about.
	 * @param State Why it is unavailable; Available shows nothing.
	 * @note Rate-limited per world, so a bone drawing every frame cannot spam the corner of the screen.
	 */
	static void NotifyUnavailable(UWorld* World, EState State);

private:
	/** Everything memoized for one world. */
	struct FWorldPreview
	{
		FNRawMesh Mesh;
		FNMeshBVH BVH;
		EState State = EState::NotBaked;
		bool bMeshValid = false;
		bool bBVHValid = false;
		uint32 Generation = 0;
		double LastNotifyTime = 0.0;
	};

	/** @return The entry for World, rebuilding the mesh and state if either is stale. */
	static FWorldPreview& EnsureBuilt(const UWorld* World);

	/** @return true when a change to Actor could alter what a collision bake would emit. */
	static bool IsRelevantActor(const AActor* Actor);

	/** Resolve a UObject reported by an editor change delegate to the AActor it belongs to (or null). */
	static AActor* ResolveActor(UObject* Object);

	/** Invalidate the world owning Actor, when Actor is collision-relevant. */
	static void InvalidateForActor(AActor* Actor);

	/** Property-change handler; ignores the continuous mid-edit stream and acts on the finalizing change. */
	static void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	static TMap<TWeakObjectPtr<const UWorld>, FWorldPreview> Previews;

	static FDelegateHandle ActorAddedHandle;
	static FDelegateHandle ActorDeletedHandle;
	static FDelegateHandle EndObjectMovementHandle;
	static FDelegateHandle PropertyChangedHandle;
	static FDelegateHandle PostUndoRedoHandle;
	static FDelegateHandle BakedHandle;
};
