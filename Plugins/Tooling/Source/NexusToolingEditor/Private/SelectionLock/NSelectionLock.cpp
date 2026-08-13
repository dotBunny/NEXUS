// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "SelectionLock/NSelectionLock.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "NEditorUtils.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"

TSet<FObjectKey> FNSelectionLock::LockedActors;
FDelegateHandle FNSelectionLock::MapOpenedHandle;
FDelegateHandle FNSelectionLock::LevelActorAddedHandle;
FDelegateHandle FNSelectionLock::ObjectsReplacedHandle;

void FNSelectionLock::Initialize()
{
	MapOpenedHandle = FEditorDelegates::OnMapOpened.AddStatic(&FNSelectionLock::OnMapOpened);
	LevelActorAddedHandle = GEngine->OnLevelActorAdded().AddStatic(&FNSelectionLock::OnLevelActorAdded);
	ObjectsReplacedHandle = FCoreUObjectDelegates::OnObjectsReplaced.AddStatic(&FNSelectionLock::OnObjectsReplaced);

	// A map is generally already open by the time the module reaches post-engine-init, and that first
	// open predates the delegate above, so it has to be picked up directly.
	CacheWorld(GEditor->GetEditorWorldContext().World());
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

	if (ObjectsReplacedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReplaced.Remove(ObjectsReplacedHandle);
		ObjectsReplacedHandle.Reset();
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
	const TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Actor);
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

	int32 UnlockedCount = 0;
	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!IsLocked(Actor))
		{
			continue;
		}

		SetLocked(Actor, false);
		++UnlockedCount;
	}

	if (UnlockedCount > 0)
	{
		GEditor->RedrawAllViewports();
	}

	return UnlockedCount;
}

void FNSelectionLock::ApplyLock(AActor* Actor, const bool bLocked)
{
	const bool bSelectable = !bLocked;

	const TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Actor);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent->bSelectable == bSelectable)
		{
			continue;
		}

		// The flag is captured into the scene proxy at creation, so the proxy has to be rebuilt for
		// the hit-proxy pass to notice.
		PrimitiveComponent->bSelectable = bSelectable;
		PrimitiveComponent->MarkRenderStateDirty();
	}
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

void FNSelectionLock::CacheWorld(UWorld* World)
{
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!ReadMetaData(Actor))
		{
			continue;
		}

		LockedActors.Add(FObjectKey(Actor));
		ApplyLock(Actor, true);
	}
}

void FNSelectionLock::OnMapOpened(const FString&, bool)
{
	LockedActors.Empty();
	CacheWorld(GEditor->GetEditorWorldContext().World());
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

	if (!ReadMetaData(Actor))
	{
		return;
	}

	LockedActors.Add(FObjectKey(Actor));
	ApplyLock(Actor, true);
}

void FNSelectionLock::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	for (const TPair<UObject*, UObject*>& Replacement : ReplacementMap)
	{
		AActor* Actor = Cast<AActor>(Replacement.Value);
		if (Actor == nullptr)
		{
			continue;
		}

		LockedActors.Remove(FObjectKey(Replacement.Key));

		// A reinstanced actor keeps its package path, so its meta-data still resolves against it.
		if (!ReadMetaData(Actor))
		{
			continue;
		}

		LockedActors.Add(FObjectKey(Actor));
		ApplyLock(Actor, true);
	}
}
