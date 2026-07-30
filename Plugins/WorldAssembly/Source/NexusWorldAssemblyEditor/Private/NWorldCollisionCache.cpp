// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionCache.h"

#include "NActorUtils.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Components/ActorComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
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

TMap<TWeakObjectPtr<const UWorld>, FNWorldCollisionCache::FAsyncWorldState> FNWorldCollisionCache::AsyncStates;
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

void FNWorldCollisionCache::MarkStateDirty(FAsyncWorldState& State)
{
	State.bAsyncDirty = true;
	++State.RebuildRequestId;
	State.LastInvalidateTime = FPlatformTime::Seconds();
	if (State.bRebuildInFlight && State.InFlightCancelFlag.IsValid())
	{
		*State.InFlightCancelFlag = true;
	}
}

void FNWorldCollisionCache::Invalidate()
{
	// Synchronous path.
	bDirty = true;
	bBVHDirty = true;
	++Generation;

	// Asynchronous path: this is the world-agnostic entry point (undo/redo), so mark every tracked world stale. Targeted
	// actor edits use InvalidateWorld and only touch the affected world.
	for (TPair<TWeakObjectPtr<const UWorld>, FAsyncWorldState>& Pair : AsyncStates)
	{
		MarkStateDirty(Pair.Value);
	}
}

void FNWorldCollisionCache::InvalidateWorld(const UWorld* World)
{
	if (World == nullptr)
	{
		return;
	}

	// Synchronous path (single cached world): only relevant when the change hit the world we currently have cached.
	if (CachedWorld.Get() == World)
	{
		bDirty = true;
		bBVHDirty = true;
		++Generation;
	}

	// Asynchronous path: mark just this world's pipeline stale, if we are tracking it. A world with no entry yet has no
	// consumer, so there is nothing to rebuild — its first RequestAsyncRefresh will start a build from scratch.
	if (FAsyncWorldState* State = AsyncStates.Find(World))
	{
		MarkStateDirty(*State);
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

	const UWorld* World = Actor->GetWorld();
	const FObjectKey Key(Actor);

	// Was it part of the geometry we last built for its world? (covers delete / collision-off / ignore-tag-added
	// transitions, where the actor no longer passes the live filter but its removal still changes that world's mesh.)
	if (World != nullptr)
	{
		if (const FAsyncWorldState* State = AsyncStates.Find(World))
		{
			if (State->SourceActors.Contains(Key))
			{
				return true;
			}
		}
		if (CachedWorld.Get() == World && CachedSourceActors.Contains(Key))
		{
			return true;
		}
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
	// a full world re-gather. Actor-scoped events invalidate only the changed actor's world. Bindings live for the
	// editor session (the cache is a process-lifetime static), so handles aren't tracked.
	if (GEngine != nullptr)
	{
		GEngine->OnLevelActorAdded().AddLambda([](AActor* Actor) { if (IsRelevantActor(Actor)) { InvalidateWorld(Actor->GetWorld()); } });
		GEngine->OnLevelActorDeleted().AddLambda([](AActor* Actor) { if (IsRelevantActor(Actor)) { InvalidateWorld(Actor->GetWorld()); } });
	}
	if (GEditor != nullptr)
	{
		GEditor->OnEndObjectMovement().AddLambda([](UObject& Object)
		{
			AActor* Actor = ResolveActor(&Object);
			if (IsRelevantActor(Actor)) { InvalidateWorld(Actor->GetWorld()); }
		});
	}

	// Undo/redo can affect geometry in ways not tied to a single reported actor (or world), so invalidate everything
	// unconditionally (as the ed mode does).
	FEditorDelegates::PostUndoRedo.AddLambda([] { Invalidate(); });

	// Skip the continuous mid-edit stream (slider scrubs, gizmo drags); rebuild on the finalizing change to a relevant
	// actor instead, scoped to that actor's world.
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([](UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
	{
		AActor* Actor = ResolveActor(Object);
		if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && IsRelevantActor(Actor))
		{
			InvalidateWorld(Actor->GetWorld());
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

	const TWeakObjectPtr<const UWorld> Key(World);
	FAsyncWorldState* State = AsyncStates.Find(Key);
	if (State == nullptr)
	{
		// First time this world is sampled: create its pipeline and schedule the initial build. Consumers see null
		// results (draw nothing) until that first background build lands. Other worlds are untouched.
		State = &AsyncStates.Add(Key);
		MarkStateDirty(*State);
	}

	// Mark that a bone is drawing for this world now — this is what keeps its pump gathering. When no bone draws for a
	// world, this stops updating and the pump leaves that world's stale cache alone until a bone reappears.
	State->LastDrawTime = FPlatformTime::Seconds();
}

TSharedPtr<const FNMeshBVH> FNWorldCollisionCache::GetPublishedBVH(const UWorld* World)
{
	const FAsyncWorldState* State = AsyncStates.Find(World);
	return State != nullptr ? State->PublishedBVH : nullptr;
}

TSharedPtr<const FNRawMesh> FNWorldCollisionCache::GetPublishedMesh(const UWorld* World)
{
	const FAsyncWorldState* State = AsyncStates.Find(World);
	return State != nullptr ? State->PublishedMesh : nullptr;
}

bool FNWorldCollisionCache::IsPublishedMeshConvex(const UWorld* World)
{
	const FAsyncWorldState* State = AsyncStates.Find(World);
	return State != nullptr && State->bPublishedMeshIsConvex;
}

bool FNWorldCollisionCache::PublishedMeshHasNonTris(const UWorld* World)
{
	const FAsyncWorldState* State = AsyncStates.Find(World);
	return State != nullptr && State->bPublishedMeshHasNonTris;
}

uint32 FNWorldCollisionCache::GetResultsGeneration(const UWorld* World)
{
	const FAsyncWorldState* State = AsyncStates.Find(World);
	return State != nullptr ? State->ResultsGeneration : 0;
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
	const double Now = FPlatformTime::Seconds();

	// Snapshot the keys so a re-entrant RequestAsyncRefresh/Invalidate during a gather (which can mutate AsyncStates and
	// reallocate it) can't invalidate the loop.
	TArray<TWeakObjectPtr<const UWorld>> Keys;
	AsyncStates.GetKeys(Keys);

	for (const TWeakObjectPtr<const UWorld>& Key : Keys)
	{
		if (Key.Get() == nullptr)
		{
			// The world was destroyed. Drain any in-flight build for it before dropping the entry so we don't dangle
			// the background task against freed state.
			if (FAsyncWorldState* State = AsyncStates.Find(Key))
			{
				if (State->bRebuildInFlight)
				{
					if (State->InFlightCancelFlag.IsValid())
					{
						*State->InFlightCancelFlag = true;
					}
					State->InFlightTask.Wait();
				}
			}
			AsyncStates.Remove(Key);
			continue;
		}

		PumpState(Key.Get(), Key, Now);
	}
}

void FNWorldCollisionCache::PumpState(const UWorld* World, const TWeakObjectPtr<const UWorld>& Key, const double Now)
{
	FAsyncWorldState* State = AsyncStates.Find(Key);
	if (State == nullptr)
	{
		return;
	}

	// 1. Publish finished work (or discard it when a newer change has superseded this build). No external calls here,
	// so the State reference stays valid across this block.
	if (State->bRebuildInFlight && State->InFlightTask.IsCompleted())
	{
		FRebuildResult Result = State->InFlightTask.GetResult();
		State->InFlightTask = UE::Tasks::TTask<FRebuildResult>();
		State->bRebuildInFlight = false;

		if (State->InFlightRequestId == State->RebuildRequestId)
		{
			State->PublishedMesh = Result.Mesh;
			State->PublishedBVH = Result.BVH;
			State->bPublishedMeshIsConvex = Result.bMeshIsConvex;
			State->bPublishedMeshHasNonTris = Result.bMeshHasNonTris;
			State->SourceActors.Reset();
			for (const FObjectKey& SourceKey : Result.SourceActors)
			{
				State->SourceActors.Add(SourceKey);
			}
			++State->ResultsGeneration;
		}
		// else: a newer edit arrived mid-build; bAsyncDirty is set, so a fresh gather starts below.
	}

	// 2. Start a new gather when stale, idle, the debounce has elapsed, and a bone actually drew recently for this
	// world — so editing collision while no bone is visible marks the cache dirty but does no gather work until a bone
	// reappears.
	if (State->bRebuildInFlight || !State->bAsyncDirty
		|| (Now - State->LastInvalidateTime) < NEXUS::WorldAssembly::CollisionCache::AsyncDebounceSeconds
		|| (Now - State->LastDrawTime) > NEXUS::WorldAssembly::CollisionCache::AsyncActiveWindowSeconds)
	{
		return;
	}
	const uint64 RequestIdAtGather = State->RebuildRequestId;

	// Game-thread gather (unavoidably: walks live actors and flushes static-mesh compilation). This can broadcast
	// editor delegates, so treat the map reference as potentially stale afterward and re-acquire before writing back.
	TArray<FNRawMesh> Meshes;
	TArray<FTransform> Transforms;
	TArray<AActor*> SourceActors;
	GatherRaw(World, Meshes, Transforms, SourceActors);

	State = AsyncStates.Find(Key);
	if (State == nullptr || State->bRebuildInFlight || State->RebuildRequestId != RequestIdAtGather)
	{
		// The world went away, another gather already started, or a newer edit superseded this one mid-gather. In the
		// superseded case bAsyncDirty is still set, so the next pump gathers afresh.
		return;
	}

	TArray<FObjectKey> SourceKeys;
	SourceKeys.Reserve(SourceActors.Num());
	for (const AActor* SourceActor : SourceActors)
	{
		SourceKeys.Add(FObjectKey(SourceActor));
	}

	// This snapshot reflects the world as of now; a later edit re-sets bAsyncDirty and supersedes via the request id.
	State->bAsyncDirty = false;
	State->InFlightRequestId = State->RebuildRequestId;
	State->InFlightCancelFlag = MakeShared<FThreadSafeBool, ESPMode::ThreadSafe>(false);
	const TSharedRef<FThreadSafeBool, ESPMode::ThreadSafe> CancelRef = State->InFlightCancelFlag.ToSharedRef();

	// Background: the merge + BVH build are pure FNRawMesh math with no UObject access.
	State->InFlightTask = UE::Tasks::Launch(TEXT("NWorldCollisionCacheRebuild"),
		[Meshes = MoveTemp(Meshes), Transforms = MoveTemp(Transforms), SourceKeys = MoveTemp(SourceKeys), CancelRef]() mutable
		{
			return MergeAndBuild(MoveTemp(Meshes), MoveTemp(Transforms), MoveTemp(SourceKeys), CancelRef);
		},
		UE::Tasks::ETaskPriority::BackgroundLow);
	State->bRebuildInFlight = true;
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

	const TWeakObjectPtr<const UWorld> Key(World);
	FAsyncWorldState* State = AsyncStates.Find(Key);
	if (State == nullptr)
	{
		return; // null world (RequestAsyncRefresh created nothing)
	}

	// Force a rebuild regardless of prior state, bypass the debounce, and mark a draw as current so the pump's
	// active-window gate lets the gather through — the pipeline then runs deterministically.
	State->bAsyncDirty = true;
	++State->RebuildRequestId;
	State->LastInvalidateTime = 0.0;
	State->LastDrawTime = FPlatformTime::Seconds();

	PumpState(World, Key, FPlatformTime::Seconds()); // gather (game thread) + launch background task

	State = AsyncStates.Find(Key);
	if (State != nullptr && State->bRebuildInFlight)
	{
		State->InFlightTask.Wait();                      // block until the background merge + BVH build finishes
		PumpState(World, Key, FPlatformTime::Seconds()); // publish
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

	// Don't leave any background task running against soon-to-be-freed state.
	for (TPair<TWeakObjectPtr<const UWorld>, FAsyncWorldState>& Pair : AsyncStates)
	{
		FAsyncWorldState& State = Pair.Value;
		if (State.bRebuildInFlight)
		{
			if (State.InFlightCancelFlag.IsValid())
			{
				*State.InFlightCancelFlag = true;
			}
			State.InFlightTask.Wait();
			State.InFlightTask = UE::Tasks::TTask<FRebuildResult>();
			State.bRebuildInFlight = false;
		}
	}

	AsyncStates.Empty();
}
