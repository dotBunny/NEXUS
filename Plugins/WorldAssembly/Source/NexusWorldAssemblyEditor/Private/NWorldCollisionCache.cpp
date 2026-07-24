// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionCache.h"

#include "NActorUtils.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Components/ActorComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Types/NRawMeshFactory.h"
#include "Types/NRawMeshUtils.h"
#include "UObject/UObjectGlobals.h"

namespace NEXUS::WorldAssembly::CollisionCache
{
	/** Coalesce a burst of edits into a single gather by waiting this long past the last invalidation. */
	constexpr double AsyncDebounceSeconds = 0.05;

	/**
	 * The pump only starts a gather while a bone visualizer drew within this window. RequestAsyncRefresh runs on every
	 * bone draw, so this stays continuously fresh while any bone is visible; when nothing is drawing, a stale cache is
	 * left alone until a bone reappears (and refreshes on demand). Comfortably covers a few dropped frames.
	 */
	constexpr double AsyncActiveWindowSeconds = 0.25;
}

FNRawMesh FNWorldCollisionCache::CachedMesh;
FNMeshBVH FNWorldCollisionCache::CachedBVH;
TWeakObjectPtr<const UWorld> FNWorldCollisionCache::CachedWorld;
TSet<FObjectKey> FNWorldCollisionCache::CachedSourceActors;
bool FNWorldCollisionCache::bDirty = true;
bool FNWorldCollisionCache::bBVHDirty = true;
bool FNWorldCollisionCache::bHooksRegistered = false;
uint32 FNWorldCollisionCache::Generation = 0;

TWeakObjectPtr<const UWorld> FNWorldCollisionCache::AsyncWorld;
TSharedPtr<const FNRawMesh> FNWorldCollisionCache::PublishedMesh;
TSharedPtr<const FNMeshBVH> FNWorldCollisionCache::PublishedBVH;
bool FNWorldCollisionCache::bPublishedMeshIsConvex = false;
bool FNWorldCollisionCache::bPublishedMeshHasNonTris = false;
uint32 FNWorldCollisionCache::ResultsGeneration = 0;
bool FNWorldCollisionCache::bAsyncDirty = false;
bool FNWorldCollisionCache::bRebuildInFlight = false;
uint64 FNWorldCollisionCache::RebuildRequestId = 0;
uint64 FNWorldCollisionCache::InFlightRequestId = 0;
UE::Tasks::TTask<FNWorldCollisionCache::FRebuildResult> FNWorldCollisionCache::InFlightTask;
TSharedPtr<FThreadSafeBool, ESPMode::ThreadSafe> FNWorldCollisionCache::InFlightCancelFlag;
double FNWorldCollisionCache::LastInvalidateTime = 0.0;
double FNWorldCollisionCache::LastDrawTime = 0.0;
bool FNWorldCollisionCache::bPumpRegistered = false;
FTSTicker::FDelegateHandle FNWorldCollisionCache::PumpHandle;

const FNRawMesh& FNWorldCollisionCache::Get(const UWorld* World)
{
	EnsureInvalidationHooks();

	if (World == nullptr)
	{
		// Only count this as a change (and drop the BVH) when we actually had a world cached — otherwise a stream of
		// null queries would churn the generation and needlessly clear every consumer's memo each frame.
		if (CachedWorld.IsValid())
		{
			++Generation;
			bBVHDirty = true;
		}
		CachedMesh = FNRawMesh();
		CachedWorld = nullptr;
		CachedSourceActors.Reset();
		bDirty = true;
		return CachedMesh;
	}

	// Rebuild when invalidated by a world change, or when queried for a different world than we last cached.
	if (bDirty || CachedWorld.Get() != World)
	{
		// A world switch changes the mesh content without going through Invalidate, so bump the generation here too.
		// (Invalidate already bumped it for the geometry-edit case.)
		if (CachedWorld.Get() != World)
		{
			++Generation;
		}

		// Capture the actors the merge gathered so the invalidation hooks can tell when a change touches geometry we
		// actually built from (e.g. a now-deleted wall that no longer passes the live filter).
		TArray<AActor*> SourceActors;
		CachedMesh = Build(World, {}, &SourceActors);
		CachedSourceActors.Reset();
		CachedSourceActors.Reserve(SourceActors.Num());
		for (const AActor* SourceActor : SourceActors)
		{
			CachedSourceActors.Add(FObjectKey(SourceActor));
		}

		CachedWorld = World;
		bDirty = false;
		bBVHDirty = true;
	}

	return CachedMesh;
}

const FNMeshBVH& FNWorldCollisionCache::GetBVH(const UWorld* World)
{
	// Ensure the mesh is current first; this may rebuild it and flag the BVH stale.
	Get(World);

	if (bBVHDirty)
	{
		CachedBVH = FNMeshBVH(CachedMesh);
		bBVHDirty = false;
	}

	return CachedBVH;
}

FNRawMesh FNWorldCollisionCache::Build(const UWorld* World, const TArray<FBoxSphereBounds>& Bounds, TArray<AActor*>* OutSourceActors)
{
	FNRawMesh MergedMesh;
	if (World == nullptr)
	{
		return MergedMesh;
	}

	// Same world-geometry definition assembly uses: filtered actors' simple collision, gathered within Bounds.
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));

	if (OutSourceActors != nullptr)
	{
		*OutSourceActors = WorldActors;
	}

	TArray<FNRawMesh> CollisionMeshes;
	TArray<FTransform> CollisionTransforms;
	FNRawMeshFactory::FromActorsInBounds(WorldActors, Bounds, CollisionMeshes, CollisionTransforms);

	// Merge every emitted mesh into one origin-anchored (world-space) mesh, mirroring the collision visualizer path.
	const FTransform MergedTransform = FTransform::Identity;
	for (int32 i = 0; i < CollisionTransforms.Num(); i++)
	{
		FNRawMeshUtils::CombineMesh(MergedTransform, MergedMesh, CollisionTransforms[i], CollisionMeshes[i]);
	}

	return MergedMesh;
}

void FNWorldCollisionCache::Invalidate()
{
	bDirty = true;
	bBVHDirty = true;
	++Generation;

	// Drive the async path: mark stale, bump the request id so any in-flight build's result is superseded, note the
	// time for the debounce, and ask the in-flight background task to bail early.
	bAsyncDirty = true;
	++RebuildRequestId;
	LastInvalidateTime = FPlatformTime::Seconds();
	if (bRebuildInFlight && InFlightCancelFlag.IsValid())
	{
		*InFlightCancelFlag = true;
	}
}

AActor* FNWorldCollisionCache::ResolveActor(UObject* Object)
{
	if (Object == nullptr)
	{
		return nullptr;
	}
	if (AActor* Actor = Cast<AActor>(Object))
	{
		return Actor;
	}
	if (const UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		return Component->GetOwner();
	}
	return nullptr;
}

bool FNWorldCollisionCache::IsRelevantActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	// Was it part of the geometry we last built? (covers delete / collision-off / ignore-tag-added transitions, where
	// the actor no longer passes the live filter but its removal still changes the mesh.)
	if (CachedSourceActors.Contains(FObjectKey(Actor)))
	{
		return true;
	}

	// Is it relevant now? (covers add / collision-on transitions.) Same predicate the merge itself uses, so a bone —
	// which is a generation input with collision disabled — is excluded and moving it never triggers a rebuild.
	return FNActorUtils::PassesFilter(Actor,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));
}

void FNWorldCollisionCache::EnsureInvalidationHooks()
{
	if (bHooksRegistered)
	{
		return;
	}
	bHooksRegistered = true;

	// Geometry changes that should rebuild the cache. Mirrors the ed mode's collision-visualizer refresh triggers —
	// including its relevance filtering, so changes to non-collision actors (bones, lights, editor gizmos) do NOT force
	// a full world re-gather. Bindings live for the editor session (the cache is a process-lifetime static), so handles
	// aren't tracked.
	if (GEngine != nullptr)
	{
		GEngine->OnLevelActorAdded().AddLambda([](AActor* Actor) { if (IsRelevantActor(Actor)) { Invalidate(); } });
		GEngine->OnLevelActorDeleted().AddLambda([](AActor* Actor) { if (IsRelevantActor(Actor)) { Invalidate(); } });
	}
	if (GEditor != nullptr)
	{
		GEditor->OnEndObjectMovement().AddLambda([](UObject& Object) { if (IsRelevantActor(ResolveActor(&Object))) { Invalidate(); } });
	}

	// Undo/redo can affect geometry in ways not tied to a single reported actor, so invalidate unconditionally (as the
	// ed mode does).
	FEditorDelegates::PostUndoRedo.AddLambda([] { Invalidate(); });

	// Skip the continuous mid-edit stream (slider scrubs, gizmo drags); rebuild on the finalizing change to a relevant
	// actor instead.
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([](UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
	{
		if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && IsRelevantActor(ResolveActor(Object)))
		{
			Invalidate();
		}
	});
}

// ----------------------------------------------------------------------------------------------------------------------
// Asynchronous path
// ----------------------------------------------------------------------------------------------------------------------

void FNWorldCollisionCache::RequestAsyncRefresh(const UWorld* World)
{
	EnsureInvalidationHooks();
	EnsureAsyncPump();

	if (World == nullptr)
	{
		return;
	}

	// Mark that a bone is drawing now — this is what keeps the pump gathering. When no bone draws, this stops updating
	// and the pump leaves a stale cache alone until a bone reappears.
	LastDrawTime = FPlatformTime::Seconds();

	if (AsyncWorld.Get() != World)
	{
		// New world (or the very first request): the previously published data belongs to a different level, so drop it
		// and rebuild from scratch. Consumers see null results (draw nothing) until the first background build lands.
		AsyncWorld = World;
		PublishedMesh.Reset();
		PublishedBVH.Reset();
		bPublishedMeshIsConvex = false;
		bPublishedMeshHasNonTris = false;
		++ResultsGeneration;

		bAsyncDirty = true;
		++RebuildRequestId;
		LastInvalidateTime = FPlatformTime::Seconds();
		if (bRebuildInFlight && InFlightCancelFlag.IsValid())
		{
			*InFlightCancelFlag = true;
		}
	}
	// Otherwise nothing to do here — the pump acts on bAsyncDirty, which Invalidate sets on relevant world changes.
}

TSharedPtr<const FNMeshBVH> FNWorldCollisionCache::GetPublishedBVH()
{
	return PublishedBVH;
}

TSharedPtr<const FNRawMesh> FNWorldCollisionCache::GetPublishedMesh()
{
	return PublishedMesh;
}

bool FNWorldCollisionCache::IsPublishedMeshConvex()
{
	return bPublishedMeshIsConvex;
}

bool FNWorldCollisionCache::PublishedMeshHasNonTris()
{
	return bPublishedMeshHasNonTris;
}

uint32 FNWorldCollisionCache::GetResultsGeneration()
{
	return ResultsGeneration;
}

void FNWorldCollisionCache::EnsureAsyncPump()
{
	if (bPumpRegistered)
	{
		return;
	}
	bPumpRegistered = true;
	PumpHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&FNWorldCollisionCache::Pump), 0.0f);
}

bool FNWorldCollisionCache::Pump(float /*DeltaTime*/)
{
	PumpOnce();
	return true; // keep ticking for the editor session
}

void FNWorldCollisionCache::PumpOnce()
{
	// 1. Publish finished work (or discard it when a newer change has superseded this build).
	if (bRebuildInFlight && InFlightTask.IsCompleted())
	{
		FRebuildResult Result = InFlightTask.GetResult();
		InFlightTask = UE::Tasks::TTask<FRebuildResult>();
		bRebuildInFlight = false;

		if (InFlightRequestId == RebuildRequestId)
		{
			PublishedMesh = Result.Mesh;
			PublishedBVH = Result.BVH;
			bPublishedMeshIsConvex = Result.bMeshIsConvex;
			bPublishedMeshHasNonTris = Result.bMeshHasNonTris;
			CachedSourceActors.Reset();
			for (const FObjectKey& SourceKey : Result.SourceActors)
			{
				CachedSourceActors.Add(SourceKey);
			}
			++ResultsGeneration;
		}
		// else: a newer edit arrived mid-build; bAsyncDirty is set, so a fresh gather starts below.
	}

	// 2. Start a new gather when stale, idle, the debounce has elapsed, and a bone actually drew recently — so editing
	// collision while no bone is visible marks the cache dirty but does no gather work until a bone reappears.
	const double Now = FPlatformTime::Seconds();
	const UWorld* World = AsyncWorld.Get();
	if (!bRebuildInFlight && bAsyncDirty && World != nullptr
		&& (Now - LastInvalidateTime) >= NEXUS::WorldAssembly::CollisionCache::AsyncDebounceSeconds
		&& (Now - LastDrawTime) <= NEXUS::WorldAssembly::CollisionCache::AsyncActiveWindowSeconds)
	{
		// Game-thread gather (unavoidably: walks live actors and flushes static-mesh compilation).
		TArray<FNRawMesh> Meshes;
		TArray<FTransform> Transforms;
		TArray<AActor*> SourceActors;
		GatherRaw(World, Meshes, Transforms, SourceActors);

		TArray<FObjectKey> SourceKeys;
		SourceKeys.Reserve(SourceActors.Num());
		for (const AActor* SourceActor : SourceActors)
		{
			SourceKeys.Add(FObjectKey(SourceActor));
		}

		// This snapshot reflects the world as of now; a later edit re-sets bAsyncDirty and supersedes via the request id.
		bAsyncDirty = false;
		InFlightRequestId = RebuildRequestId;
		InFlightCancelFlag = MakeShared<FThreadSafeBool, ESPMode::ThreadSafe>(false);
		const TSharedRef<FThreadSafeBool, ESPMode::ThreadSafe> CancelRef = InFlightCancelFlag.ToSharedRef();

		// Background: the merge + BVH build are pure FNRawMesh math with no UObject access.
		InFlightTask = UE::Tasks::Launch(TEXT("NWorldCollisionCacheRebuild"),
			[Meshes = MoveTemp(Meshes), Transforms = MoveTemp(Transforms), SourceKeys = MoveTemp(SourceKeys), CancelRef]() mutable
			{
				return MergeAndBuild(MoveTemp(Meshes), MoveTemp(Transforms), MoveTemp(SourceKeys), CancelRef);
			},
			UE::Tasks::ETaskPriority::BackgroundLow);
		bRebuildInFlight = true;
	}
}

void FNWorldCollisionCache::GatherRaw(const UWorld* World, TArray<FNRawMesh>& OutMeshes, TArray<FTransform>& OutTransforms,
	TArray<AActor*>& OutSourceActors)
{
	OutMeshes.Reset();
	OutTransforms.Reset();
	OutSourceActors.Reset();

	if (World == nullptr)
	{
		return;
	}

	// Same world-geometry definition the synchronous Build uses, but left unmerged so the merge can run off-thread.
	OutSourceActors = FNActorUtils::GetWorldActors(World,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));
	FNRawMeshFactory::FromActorsInBounds(OutSourceActors, {}, OutMeshes, OutTransforms);
}

FNWorldCollisionCache::FRebuildResult FNWorldCollisionCache::MergeAndBuild(TArray<FNRawMesh> Meshes,
	TArray<FTransform> Transforms, TArray<FObjectKey> SourceActors, TSharedRef<FThreadSafeBool, ESPMode::ThreadSafe> Cancel)
{
	FRebuildResult Result;

	// Merge every emitted mesh into one origin-anchored (world-space) mesh, mirroring the synchronous Build path. Poll
	// the cancel flag between merges so a superseded build bails promptly instead of finishing wasted work.
	FNRawMesh Merged;
	const FTransform MergedTransform = FTransform::Identity;
	for (int32 i = 0; i < Transforms.Num(); ++i)
	{
		if (*Cancel)
		{
			return Result; // empty; discarded on completion because the request id will have moved on
		}
		FNRawMeshUtils::CombineMesh(MergedTransform, Merged, Transforms[i], Meshes[i]);
	}

	if (*Cancel)
	{
		return Result;
	}

	// Validate here (off the game thread) so the published mesh's convexity / non-tri flags are populated up front and
	// the game-thread consumers never trigger the lazy validation on the shared mesh.
	Merged.Validate();
	Result.bMeshIsConvex = Merged.IsConvex();
	Result.bMeshHasNonTris = Merged.HasNonTris();

	const TSharedRef<FNMeshBVH> BuiltBVH = MakeShared<FNMeshBVH>(Merged);
	Result.Mesh = MakeShared<FNRawMesh>(MoveTemp(Merged));
	Result.BVH = BuiltBVH;
	Result.SourceActors = MoveTemp(SourceActors);
	return Result;
}

void FNWorldCollisionCache::FlushAsyncRefreshForTesting(const UWorld* World)
{
	RequestAsyncRefresh(World);

	// Force a rebuild regardless of prior state, bypass the debounce, and mark a draw as current so the pump's
	// active-window gate lets the gather through — the pipeline then runs deterministically.
	bAsyncDirty = true;
	++RebuildRequestId;
	LastInvalidateTime = 0.0;
	LastDrawTime = FPlatformTime::Seconds();

	PumpOnce(); // gather (game thread) + launch background task
	if (bRebuildInFlight)
	{
		InFlightTask.Wait(); // block until the background merge + BVH build finishes
		PumpOnce();          // publish
	}
}

void FNWorldCollisionCache::Shutdown()
{
	if (bPumpRegistered)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PumpHandle);
		PumpHandle = FTSTicker::FDelegateHandle();
		bPumpRegistered = false;
	}

	// Don't leave a background task running against soon-to-be-freed state.
	if (bRebuildInFlight)
	{
		if (InFlightCancelFlag.IsValid())
		{
			*InFlightCancelFlag = true;
		}
		InFlightTask.Wait();
		InFlightTask = UE::Tasks::TTask<FRebuildResult>();
		bRebuildInFlight = false;
	}

	PublishedMesh.Reset();
	PublishedBVH.Reset();
	InFlightCancelFlag.Reset();
	AsyncWorld = nullptr;
}
