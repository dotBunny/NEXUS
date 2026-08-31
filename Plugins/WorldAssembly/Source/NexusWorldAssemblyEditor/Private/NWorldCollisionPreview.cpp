// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionPreview.h"

#include "Editor.h"
#include "NActorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionBaker.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Types/NRawMeshUtils.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace NEXUS::WorldAssembly::CollisionPreview
{
	/** Shortest gap between two "cannot preview" notifications for the same world. */
	constexpr double NotifyCooldownSeconds = 10.0;
}

TMap<TWeakObjectPtr<const UWorld>, FNWorldCollisionPreview::FWorldPreview> FNWorldCollisionPreview::Previews;

FDelegateHandle FNWorldCollisionPreview::ActorAddedHandle;
FDelegateHandle FNWorldCollisionPreview::ActorDeletedHandle;
FDelegateHandle FNWorldCollisionPreview::EndObjectMovementHandle;
FDelegateHandle FNWorldCollisionPreview::PropertyChangedHandle;
FDelegateHandle FNWorldCollisionPreview::PostUndoRedoHandle;
FDelegateHandle FNWorldCollisionPreview::BakedHandle;

void FNWorldCollisionPreview::Register()
{
	// The same triggers the save-time bake watches, filtered the same way — so moving a bone or a light, which changes
	// no collision geometry, costs the preview nothing. Bones in particular: they are the thing most often dragged
	// while the penetration readout is on screen, and they never invalidate it.
	if (GEngine != nullptr)
	{
		ActorAddedHandle = GEngine->OnLevelActorAdded().AddStatic(&FNWorldCollisionPreview::InvalidateForActor);
		ActorDeletedHandle = GEngine->OnLevelActorDeleted().AddStatic(&FNWorldCollisionPreview::InvalidateForActor);
	}

	if (GEditor != nullptr)
	{
		EndObjectMovementHandle = GEditor->OnEndObjectMovement().AddLambda([](UObject& Object)
		{
			InvalidateForActor(ResolveActor(&Object));
		});
	}

	PropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(
		&FNWorldCollisionPreview::OnObjectPropertyChanged);

	// Undo/redo can move geometry in ways not tied to any one reported actor or world.
	PostUndoRedoHandle = FEditorDelegates::PostUndoRedo.AddLambda([] { Invalidate(); });

	// A bake is the only thing that changes what the pool holds, and it changes no actor — so none of the delegates
	// above see it. Subscribed here rather than invalidating from each bake site, which is what let a freshly baked
	// level keep reporting the pre-bake answer: the rail, the details panel and the save hook all bake, and each was
	// a separate place to remember.
	BakedHandle = FNWorldCollisionBaker::OnBaked.AddStatic(&FNWorldCollisionPreview::Invalidate);
}

void FNWorldCollisionPreview::Unregister()
{
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

	FNWorldCollisionBaker::OnBaked.Remove(BakedHandle);
	BakedHandle.Reset();

	Previews.Reset();
}

AActor* FNWorldCollisionPreview::ResolveActor(UObject* Object)
{
	if (Object == nullptr) return nullptr;
	if (AActor* Actor = Cast<AActor>(Object)) return Actor;
	if (const UActorComponent* Component = Cast<UActorComponent>(Object)) return Component->GetOwner();
	return nullptr;
}

bool FNWorldCollisionPreview::IsRelevantActor(const AActor* Actor)
{
	if (!IsValid(Actor)) return false;

	return FNActorUtils::PassesFilter(Actor,
		FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));
}

void FNWorldCollisionPreview::InvalidateForActor(AActor* Actor)
{
	if (!IsRelevantActor(Actor)) return;
	Invalidate(Actor->GetWorld());
}

void FNWorldCollisionPreview::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Ignore the continuous mid-edit stream (slider scrubs, gizmo drags); the finalizing change is what counts.
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive) return;

	InvalidateForActor(ResolveActor(Object));
}

void FNWorldCollisionPreview::Invalidate(const UWorld* World)
{
	if (World == nullptr)
	{
		for (TPair<TWeakObjectPtr<const UWorld>, FWorldPreview>& Pair : Previews)
		{
			Pair.Value.bMeshValid = false;
			Pair.Value.bBVHValid = false;
			++Pair.Value.Generation;
		}
		return;
	}

	if (FWorldPreview* Preview = Previews.Find(World))
	{
		Preview->bMeshValid = false;
		Preview->bBVHValid = false;
		++Preview->Generation;
	}
}

FNWorldCollisionPreview::FWorldPreview& FNWorldCollisionPreview::EnsureBuilt(const UWorld* World)
{
	FWorldPreview& Preview = Previews.FindOrAdd(World);
	if (Preview.bMeshValid)
	{
		return Preview;
	}

	const FNWorldAssemblyWorldCollisionSettings& Settings = UNWorldAssemblySettings::Get()->WorldCollisionSettings;
	const TArray<UNOrganComponent*> Organs = FNWorldCollisionBaker::GetWorldOrgans(World);

	// The one validation pass, paid here rather than per query. Geometry edits invalidate the memo, so this runs once
	// per completed edit — not once per viewport redraw, which is what made the old async pipeline necessary.
	FNWorldCollisionBaker::FPooledCollision Pooled = FNWorldCollisionBaker::ResolvePooled(World, Settings, Organs,
		true, true);

	if (!Pooled.bPoolUsable)
	{
		// No cache actor, an empty pool, or no organs to address it through — all of which read as "never baked", and
		// the one case with genuinely nothing to show.
		Preview.State = EState::NotBaked;
		Preview.Mesh = FNRawMesh();
		Preview.bBVHValid = false;
	}
	else if (!Pooled.IsComplete() || !Pooled.bLandscapeResolved)
	{
		// Stale keeps whatever was last built rather than clearing it. Every world-geometry edit lands here, and a
		// visualizer that blanked on each one would be unusable while authoring — where showing the last baked state
		// alongside a notification is both useful and honest. A partial rebuild is still refused: drawing some of the
		// level's collision and silently omitting the rest is worse than showing a known-old whole.
		Preview.State = EState::Stale;
	}
	else
	{
		Preview.State = EState::Available;
		Preview.Mesh = FNRawMesh();
		Preview.bBVHValid = false;

		// Already baked — world-space, convex, face planes warmed — so they merge in with an identity transform.
		const FTransform MergedTransform = FTransform::Identity;
		for (const FNRawMesh& Mesh : Pooled.Meshes)
		{
			FNRawMeshUtils::CombineMesh(MergedTransform, Preview.Mesh, FTransform::Identity, Mesh);
		}
	}

	Preview.bMeshValid = true;
	return Preview;
}

const FNRawMesh& FNWorldCollisionPreview::GetMesh(const UWorld* World)
{
	static const FNRawMesh EmptyMesh;
	if (World == nullptr) return EmptyMesh;

	return EnsureBuilt(World).Mesh;
}

const FNMeshBVH& FNWorldCollisionPreview::GetBVH(const UWorld* World)
{
	static const FNMeshBVH EmptyBVH;
	if (World == nullptr) return EmptyBVH;

	FWorldPreview& Preview = EnsureBuilt(World);
	if (!Preview.bBVHValid)
	{
		// Built lazily, so a level whose bones are never sampled never pays for it.
		Preview.BVH = FNMeshBVH(Preview.Mesh);
		Preview.bBVHValid = true;
	}

	return Preview.BVH;
}

FNWorldCollisionPreview::EState FNWorldCollisionPreview::GetState(const UWorld* World)
{
	if (World == nullptr) return EState::NotBaked;

	return EnsureBuilt(World).State;
}

uint32 FNWorldCollisionPreview::GetGeneration(const UWorld* World)
{
	if (World == nullptr) return 0;

	return Previews.FindOrAdd(World).Generation;
}

void FNWorldCollisionPreview::NotifyUnavailable(UWorld* World, const EState State)
{
	if (World == nullptr || State == EState::Available) return;

	FWorldPreview& Preview = Previews.FindOrAdd(World);

	// Rate-limited because the bone readout asks on every redraw. Without this, an unbaked level with a bone selected
	// would stack a notification per frame.
	const double Now = FPlatformTime::Seconds();
	if (Now - Preview.LastNotifyTime < NEXUS::WorldAssembly::CollisionPreview::NotifyCooldownSeconds)
	{
		return;
	}
	Preview.LastNotifyTime = Now;

	FNotificationInfo Info(State == EState::Stale
		? NSLOCTEXT("NexusWorldAssemblyEditor", "CollisionPreviewStale", "World collision cache is out of date")
		: NSLOCTEXT("NexusWorldAssemblyEditor", "CollisionPreviewNotBaked", "World collision has not been baked"));

	Info.SubText = State == EState::Stale
		? NSLOCTEXT("NexusWorldAssemblyEditor", "CollisionPreviewStaleSub",
			"The level changed since it was last baked, so collision cannot be previewed. Bake to bring it up to date.")
		: NSLOCTEXT("NexusWorldAssemblyEditor", "CollisionPreviewNotBakedSub",
			"Bake the level's world collision to preview it. Saving the level bakes it too.");

	Info.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
	Info.ExpireDuration = 8.0f;
	Info.bFireAndForget = true;

	// The offered action, so the notification is a way out rather than only a complaint. A forced bake, because
	// someone acting on this has just been told the stored answer cannot be trusted.
	Info.ButtonDetails.Add(FNotificationButtonInfo(
		NSLOCTEXT("NexusWorldAssemblyEditor", "CollisionPreviewBake", "Bake"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "CollisionPreviewBakeTooltip", "Bake this level's world collision now."),
		FSimpleDelegate::CreateLambda([WeakWorld = TWeakObjectPtr<UWorld>(World)]()
		{
			UWorld* BakeWorld = WeakWorld.Get();
			if (BakeWorld == nullptr) return;

			FNWorldCollisionBaker::BakeWorld(BakeWorld, UNWorldAssemblySettings::Get()->WorldCollisionSettings, true);
			Invalidate(BakeWorld);
		})));

	FSlateNotificationManager::Get().AddNotification(Info);
}
