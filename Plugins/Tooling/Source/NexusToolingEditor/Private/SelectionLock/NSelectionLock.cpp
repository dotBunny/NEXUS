// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "SelectionLock/NSelectionLock.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "NEditorUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/MetaData.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/Package.h"

/**
 * May this component be handed back to selectable?
 * Components that ship unselectable - a nav-mesh renderer, an ISM pool's debug draw - say so on their
 * archetype, and unlocking an actor that owns one must not quietly make it clickable.
 * @param PrimitiveComponent The component to test.
 * @return true when the component's archetype allows selection, meaning the lock is what cleared it.
 */
static bool NSelectionLockCanRestoreSelectable(const UPrimitiveComponent* PrimitiveComponent)
{
	const UPrimitiveComponent* Archetype = Cast<UPrimitiveComponent>(PrimitiveComponent->GetArchetype());
	return Archetype == nullptr || Archetype->bSelectable;
}

TSet<FObjectKey> FNSelectionLock::LockedActors;
FDelegateHandle FNSelectionLock::MapOpenedHandle;
FDelegateHandle FNSelectionLock::LevelActorAddedHandle;
FDelegateHandle FNSelectionLock::LevelAddedToWorldHandle;
FDelegateHandle FNSelectionLock::ObjectsReplacedHandle;
FDelegateHandle FNSelectionLock::PreSaveWorldHandle;
FDelegateHandle FNSelectionLock::PostSaveWorldHandle;
FDelegateHandle FNSelectionLock::PostUndoRedoHandle;

void FNSelectionLock::Initialize()
{
	MapOpenedHandle = FEditorDelegates::OnMapOpened.AddStatic(&FNSelectionLock::OnMapOpened);
	LevelActorAddedHandle = GEngine->OnLevelActorAdded().AddStatic(&FNSelectionLock::OnLevelActorAdded);
	LevelAddedToWorldHandle = FWorldDelegates::LevelAddedToWorld.AddStatic(&FNSelectionLock::OnLevelAddedToWorld);
	ObjectsReplacedHandle = FCoreUObjectDelegates::OnObjectsReplaced.AddStatic(&FNSelectionLock::OnObjectsReplaced);
	PreSaveWorldHandle = FEditorDelegates::PreSaveWorldWithContext.AddStatic(&FNSelectionLock::OnPreSaveWorld);
	PostSaveWorldHandle = FEditorDelegates::PostSaveWorldWithContext.AddStatic(&FNSelectionLock::OnPostSaveWorld);
	PostUndoRedoHandle = FEditorDelegates::PostUndoRedo.AddStatic(&FNSelectionLock::OnPostUndoRedo);

	// The editor world at post-engine-init is the empty world UEditorEngine::InitEditor stands up, not
	// the startup map: EditorInit runs EngineLoop.Init (which broadcasts post-engine-init) before
	// FUnrealEdMisc::OnInit loads the map, so the startup map arrives through OnMapOpened like any
	// other. Seeding here only covers a world already being up, and costs one empty sweep otherwise.
	RebuildForWorld(GEditor->GetEditorWorldContext().World());
}

void FNSelectionLock::Shutdown()
{
	if (MapOpenedHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(MapOpenedHandle);
		MapOpenedHandle.Reset();
	}

	if (LevelActorAddedHandle.IsValid())
	{
		// GEngine is cleared in UEngine::FinishDestroy during the exit-time GC purge, which runs well
		// before modules unload, so the delegate it owns cannot be reached from here at editor exit.
		// A live-coding reload unloads the module with the engine still up, and there it does need
		// releasing — otherwise the delegate would fire into unloaded code.
		if (!FNEditorUtils::IsEditorShuttingDown())
		{
			GEngine->OnLevelActorAdded().Remove(LevelActorAddedHandle);
		}

		LevelActorAddedHandle.Reset();
	}

	if (LevelAddedToWorldHandle.IsValid())
	{
		FWorldDelegates::LevelAddedToWorld.Remove(LevelAddedToWorldHandle);
		LevelAddedToWorldHandle.Reset();
	}

	if (ObjectsReplacedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReplaced.Remove(ObjectsReplacedHandle);
		ObjectsReplacedHandle.Reset();
	}

	if (PreSaveWorldHandle.IsValid())
	{
		FEditorDelegates::PreSaveWorldWithContext.Remove(PreSaveWorldHandle);
		PreSaveWorldHandle.Reset();
	}

	if (PostSaveWorldHandle.IsValid())
	{
		FEditorDelegates::PostSaveWorldWithContext.Remove(PostSaveWorldHandle);
		PostSaveWorldHandle.Reset();
	}

	if (PostUndoRedoHandle.IsValid())
	{
		FEditorDelegates::PostUndoRedo.Remove(PostUndoRedoHandle);
		PostUndoRedoHandle.Reset();
	}

	LockedActors.Empty();
}

bool FNSelectionLock::CanLock(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	// Without a primitive there is no hit proxy to suppress, so the lock would have nothing to act on.
	const TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Actor, true);
	return !PrimitiveComponents.IsEmpty();
}

bool FNSelectionLock::IsLocked(const AActor* Actor)
{
	return Actor != nullptr && LockedActors.Contains(FObjectKey(Actor));
}

void FNSelectionLock::SetLocked(AActor* Actor, const bool bLocked)
{
	if (!CanLock(Actor) || IsLocked(Actor) == bLocked)
	{
		return;
	}

	if (bLocked)
	{
		LockedActors.Add(FObjectKey(Actor));
	}
	else
	{
		LockedActors.Remove(FObjectKey(Actor));
	}

	WriteMetaData(Actor, bLocked);
	ApplyLock(Actor, bLocked);
}

void FNSelectionLock::ToggleLocked(const TArray<AActor*>& Actors)
{
	// A mixed selection locks rather than splitting, matching how the visibility gutter treats a group.
	bool bAllLocked = true;
	for (const AActor* Actor : Actors)
	{
		if (CanLock(Actor) && !IsLocked(Actor))
		{
			bAllLocked = false;
			break;
		}
	}

	for (AActor* Actor : Actors)
	{
		SetLocked(Actor, !bAllLocked);
	}

	GEditor->RedrawAllViewports();
}

int32 FNSelectionLock::UnlockAll(UWorld* World)
{
	if (World == nullptr)
	{
		return 0;
	}

	// Resolved up front rather than iterated live, since SetLocked mutates the cache underneath.
	const TArray<AActor*> Actors = GetLockedActors(World);
	for (AActor* Actor : Actors)
	{
		SetLocked(Actor, false);
	}

	if (!Actors.IsEmpty())
	{
		GEditor->RedrawAllViewports();
	}

	return Actors.Num();
}

TArray<AActor*> FNSelectionLock::GetLockedActors(const UWorld* World)
{
	TArray<AActor*> Actors;
	Actors.Reserve(LockedActors.Num());

	for (TSet<FObjectKey>::TIterator KeyIterator(LockedActors); KeyIterator; ++KeyIterator)
	{
		AActor* Actor = Cast<AActor>(KeyIterator->ResolveObjectPtr());
		if (Actor == nullptr)
		{
			// Deleted, or left behind by a map the cache has already moved past.
			KeyIterator.RemoveCurrent();
			continue;
		}

		if (World == nullptr || Actor->GetWorld() == World)
		{
			Actors.Add(Actor);
		}
	}

	return Actors;
}

bool FNSelectionLock::ApplyLock(AActor* Actor, const bool bLocked)
{
	const bool bSelectable = !bLocked;

	bool bChanged = false;
	const TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Actor, true);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent->bSelectable == bSelectable)
		{
			continue;
		}

		if (bSelectable && !NSelectionLockCanRestoreSelectable(PrimitiveComponent))
		{
			continue;
		}

		// The flag is captured into the scene proxy at creation, so the proxy has to be rebuilt for
		// the hit-proxy pass to notice.
		PrimitiveComponent->bSelectable = bSelectable;
		PrimitiveComponent->MarkRenderStateDirty();
		bChanged = true;
	}

	return bChanged;
}

bool FNSelectionLock::ReadMetaData(const AActor* Actor)
{
	UPackage* Package = Actor->GetPackage();
	if (Package == nullptr)
	{
		return false;
	}

	return Package->GetMetaData().HasValue(Actor, NEXUS::ToolingEditor::SelectionLock::MetaKey);
}

void FNSelectionLock::WriteMetaData(const AActor* Actor, const bool bLocked)
{
	UPackage* Package = Actor->GetPackage();
	if (Package == nullptr)
	{
		return;
	}

	FMetaData& MetaData = Package->GetMetaData();
	if (bLocked)
	{
		MetaData.SetValue(Actor, NEXUS::ToolingEditor::SelectionLock::MetaKey, NEXUS::ToolingEditor::SelectionLock::MetaValue);
	}
	else
	{
		MetaData.RemoveValue(Actor, NEXUS::ToolingEditor::SelectionLock::MetaKey);
	}

	// FMetaData does not dirty its package on write, so without this the change never reaches disk.
	Package->MarkPackageDirty();
}

bool FNSelectionLock::RestoreActor(AActor* Actor)
{
	if (!ReadMetaData(Actor))
	{
		LockedActors.Remove(FObjectKey(Actor));
		return false;
	}

	LockedActors.Add(FObjectKey(Actor));
	return ApplyLock(Actor, true);
}

void FNSelectionLock::RestoreWorld(UWorld* World)
{
	if (World == nullptr)
	{
		return;
	}

	bool bChanged = false;
	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		bChanged |= RestoreActor(*ActorIterator);
	}

	if (bChanged)
	{
		// A render-state change does not drop a viewport's cached hit-proxy map, so the redraw is what
		// makes the restored lock take effect on the next click rather than the next invalidation.
		GEditor->RedrawAllViewports();
	}
}

void FNSelectionLock::RestoreLevel(ULevel* Level)
{
	if (Level == nullptr)
	{
		return;
	}

	bool bChanged = false;
	for (AActor* Actor : Level->Actors)
	{
		if (Actor != nullptr)
		{
			bChanged |= RestoreActor(Actor);
		}
	}

	if (bChanged)
	{
		GEditor->RedrawAllViewports();
	}
}

void FNSelectionLock::RebuildForWorld(UWorld* World)
{
	LockedActors.Empty();
	RestoreWorld(World);
}

void FNSelectionLock::SetLocksApplied(UWorld* World, const bool bApplied)
{
	if (World == nullptr)
	{
		return;
	}

	for (AActor* Actor : GetLockedActors(World))
	{
		const TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Actor, true);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (bApplied)
			{
				PrimitiveComponent->bSelectable = false;
			}
			else if (NSelectionLockCanRestoreSelectable(PrimitiveComponent))
			{
				PrimitiveComponent->bSelectable = true;
			}
		}
	}
}

void FNSelectionLock::OnMapOpened(const FString&, bool)
{
	RebuildForWorld(GEditor->GetEditorWorldContext().World());
}

void FNSelectionLock::OnLevelActorAdded(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return;
	}

	// Fires for PIE and preview worlds too, neither of which has a viewport this lock applies to.
	const UWorld* World = Actor->GetWorld();
	if (World == nullptr || World->WorldType != EWorldType::Editor)
	{
		return;
	}

	if (RestoreActor(Actor))
	{
		GEditor->RedrawAllViewports();
	}
}

void FNSelectionLock::OnLevelAddedToWorld(ULevel* Level, UWorld* World)
{
	if (World == nullptr || World->WorldType != EWorldType::Editor)
	{
		return;
	}

	RestoreLevel(Level);
}

void FNSelectionLock::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	// Two shapes arrive here. Blueprint reinstancing replaces the actor itself, and the cache has to be
	// re-keyed onto the new object. A construction-script rerun leaves the actor alone but replaces its
	// components, and bSelectable cannot survive that: it is a bare UPROPERTY carrying neither CPF_Edit
	// nor CPF_Interp, which is exactly what FComponentInstanceDataCache skips, so the rebuilt components
	// come back at the class default of selectable and the lock is silently gone.
	TSet<AActor*> ActorsToRestore;

	for (const TPair<UObject*, UObject*>& Replacement : ReplacementMap)
	{
		if (AActor* Actor = Cast<AActor>(Replacement.Value))
		{
			LockedActors.Remove(FObjectKey(Replacement.Key));

			// A reinstanced actor keeps its package path, so its meta-data still resolves against it.
			if (ReadMetaData(Actor))
			{
				LockedActors.Add(FObjectKey(Actor));
				ActorsToRestore.Add(Actor);
			}

			continue;
		}

		const UActorComponent* Component = Cast<UActorComponent>(Replacement.Value);
		if (Component == nullptr)
		{
			continue;
		}

		// A rerun replaces every construction-script component the actor owns, so collecting owners
		// keeps this to one pass over each actor's component list rather than one per entry.
		AActor* Owner = Component->GetOwner();
		if (IsLocked(Owner))
		{
			ActorsToRestore.Add(Owner);
		}
	}

	bool bChanged = false;
	for (AActor* Actor : ActorsToRestore)
	{
		bChanged |= ApplyLock(Actor, true);
	}

	if (bChanged)
	{
		GEditor->RedrawAllViewports();
	}
}

void FNSelectionLock::OnPreSaveWorld(UWorld* World, FObjectPreSaveContext)
{
	// bSelectable is a plain saved UPROPERTY, so a locked actor would otherwise carry the cleared flag
	// into the package, giving the lock a second persistence channel that can drift from the meta-data.
	// Render state is deliberately left alone: the flag is only read when a scene proxy is built, and
	// it is back on the components before anything gets a chance to rebuild one.
	SetLocksApplied(World, false);
}

void FNSelectionLock::OnPostSaveWorld(UWorld* World, FObjectPostSaveContext)
{
	SetLocksApplied(World, true);
}

void FNSelectionLock::OnPostUndoRedo()
{
	// Undo restores whatever a transaction captured, and a transaction that opened before the actor was
	// locked captured bSelectable as set. The meta-data is not transacted, so the cache is still right
	// and re-asserting from it is what keeps the two in step.
	bool bChanged = false;
	for (AActor* Actor : GetLockedActors(nullptr))
	{
		bChanged |= ApplyLock(Actor, true);
	}

	if (bChanged)
	{
		GEditor->RedrawAllViewports();
	}
}
