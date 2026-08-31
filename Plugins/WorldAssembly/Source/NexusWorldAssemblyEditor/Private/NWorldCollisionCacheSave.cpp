// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionCacheSave.h"

#include "Editor.h"
#include "NActorUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionBaker.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectSaveContext.h"

TSet<TWeakObjectPtr<const UWorld>> FNWorldCollisionCacheSave::CleanWorlds;

FDelegateHandle FNWorldCollisionCacheSave::PreSaveWorldHandle;
FDelegateHandle FNWorldCollisionCacheSave::ActorAddedHandle;
FDelegateHandle FNWorldCollisionCacheSave::ActorDeletedHandle;
FDelegateHandle FNWorldCollisionCacheSave::EndObjectMovementHandle;
FDelegateHandle FNWorldCollisionCacheSave::PropertyChangedHandle;
FDelegateHandle FNWorldCollisionCacheSave::PostUndoRedoHandle;

void FNWorldCollisionCacheSave::Register()
{
	PreSaveWorldHandle = FEditorDelegates::PreSaveWorldWithContext.AddStatic(&FNWorldCollisionCacheSave::OnPreSaveWorld);

	// The same triggers FNWorldCollisionCache watches, and filtered the same way — so moving a bone or a light, which
	// changes no collision geometry, does not cost the next save a fingerprint pass.
	if (GEngine != nullptr)
	{
		ActorAddedHandle = GEngine->OnLevelActorAdded().AddStatic(&FNWorldCollisionCacheSave::MarkDirtyForActor);
		ActorDeletedHandle = GEngine->OnLevelActorDeleted().AddStatic(&FNWorldCollisionCacheSave::MarkDirtyForActor);
	}

	if (GEditor != nullptr)
	{
		EndObjectMovementHandle = GEditor->OnEndObjectMovement().AddLambda([](UObject& Object)
		{
			MarkDirtyForActor(ResolveActor(&Object));
		});
	}

	PropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(
		&FNWorldCollisionCacheSave::OnObjectPropertyChanged);

	// Undo/redo can move geometry in ways not tied to any one reported actor or world, so nothing is assumed clean
	// afterwards. Every world falls back to its fingerprint pass, which is the authority anyway.
	PostUndoRedoHandle = FEditorDelegates::PostUndoRedo.AddLambda([] { CleanWorlds.Reset(); });
}

void FNWorldCollisionCacheSave::Unregister()
{
	FEditorDelegates::PreSaveWorldWithContext.Remove(PreSaveWorldHandle);
	PreSaveWorldHandle.Reset();

	if (GEngine != nullptr)
	{
		GEngine->OnLevelActorAdded().Remove(ActorAddedHandle);
		GEngine->OnLevelActorDeleted().Remove(ActorDeletedHandle);
	}
	ActorAddedHandle.Reset();
	ActorDeletedHandle.Reset();

	if (GEditor != nullptr)
	{
		GEditor->OnEndObjectMovement().Remove(EndObjectMovementHandle);
	}
	EndObjectMovementHandle.Reset();

	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyChangedHandle);
	PropertyChangedHandle.Reset();

	FEditorDelegates::PostUndoRedo.Remove(PostUndoRedoHandle);
	PostUndoRedoHandle.Reset();

	CleanWorlds.Reset();
}

AActor* FNWorldCollisionCacheSave::ResolveActor(UObject* Object)
{
	if (Object == nullptr) return nullptr;
	if (AActor* Actor = Cast<AActor>(Object)) return Actor;
	if (const UActorComponent* Component = Cast<UActorComponent>(Object)) return Component->GetOwner();
	return nullptr;
}

bool FNWorldCollisionCacheSave::IsRelevantActor(const AActor* Actor)
{
	if (!IsValid(Actor)) return false;

	// Asks only whether the actor is collision geometry now, where the ed mode's equivalent also asks whether it was
	// part of the last build. That second half would catch a deletion the filter can no longer recognize; here it is
	// not worth keeping a per-world source set for, because being wrong costs one fingerprint pass on the next save
	// rather than a stale visualizer.
	return FNActorUtils::PassesFilter(Actor,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));
}

void FNWorldCollisionCacheSave::MarkDirtyForActor(AActor* Actor)
{
	if (!IsRelevantActor(Actor)) return;
	MarkDirty(Actor->GetWorld());
}

void FNWorldCollisionCacheSave::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Ignore the continuous mid-edit stream (slider scrubs, gizmo drags); the finalizing change is what counts.
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive) return;

	MarkDirtyForActor(ResolveActor(Object));
}

void FNWorldCollisionCacheSave::MarkDirty(const UWorld* World)
{
	if (World == nullptr) return;
	CleanWorlds.Remove(World);
}

void FNWorldCollisionCacheSave::MarkClean(const UWorld* World)
{
	if (World == nullptr) return;
	CleanWorlds.Add(World);
}

bool FNWorldCollisionCacheSave::IsDirty(const UWorld* World)
{
	if (World == nullptr) return false;

	// Absent means dirty. A world this session has not watched from the start may have been edited by something that
	// never fired these delegates, and treating that as clean would ship a stale cache.
	return !CleanWorlds.Contains(World);
}

void FNWorldCollisionCacheSave::OnPreSaveWorld(UWorld* World, FObjectPreSaveContext ObjectPreSaveContext)
{
	// Baking is editor-only authoring. Doing it mid-cook would spawn the cache actor into a cooked package, which the
	// cooker flags as an unexpected load — the same reason the cell side-car bails here.
	if (ObjectPreSaveContext.IsCooking()) return;

	if (World == nullptr) return;

	if (!UNWorldAssemblySettings::Get()->CollisionCacheSettings.bCacheOnSave) return;

	// The cheap exit: nothing collision-relevant has been touched since this world was last baked, so there is no
	// need even to fingerprint it.
	if (!IsDirty(World)) return;

	const FNWorldCollisionBaker::FBakeResult Result = FNWorldCollisionBaker::BakeWorld(World,
		UNWorldAssemblySettings::Get()->WorldCollisionSettings, false);

	// Clean only when the bake ran to completion: it has then fingerprinted every organ against the live world, so
	// whatever it decided is current as of now — including deciding nothing needed doing. A cancelled bake never
	// reached some organs, and calling it clean would skip them on every future save until something else edits them.
	if (!Result.bCancelled)
	{
		MarkClean(World);
	}

	if (Result.bChanged)
	{
		UE_LOG(LogNexusWorldAssembly, Log, TEXT("Baked world collision for %d organ(s) while saving '%s'%s."),
			Result.OrgansBaked, *World->GetName(),
			Result.bCancelled ? TEXT(" (cancelled before finishing)") : TEXT(""));
	}
}
