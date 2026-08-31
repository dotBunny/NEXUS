// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionCacheActor.h"

#include "NWorldAssemblyMinimal.h"
#include "Components/BillboardComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"

ANWorldCollisionCacheActor::ANWorldCollisionCacheActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Pure data. Never relevant to a client — the pool is an authoring artifact consumed during assembly, and the
	// results of that assembly are what replicate.
	bReplicates = false;
	SetHidden(true);
	SetCanBeDamaged(false);

	// Always loaded under World Partition, as ANOrganVolume is. The pool describes the whole level and is asked for by
	// name rather than found by location, so leaving it spatially loaded ties whether a level appears baked at all to
	// whether the region around the world origin — where this is spawned — happens to be loaded.
	bIsSpatiallyLoaded = false;

#if WITH_EDITORONLY_DATA
	// AInfo builds a billboard and roots the actor on it, which puts an info icon at the world origin of every baked
	// level. There is nothing to look at or click here — the actor is a container for data nobody edits by hand, found
	// by name rather than by eye — so the icon is only something to trip over in the viewport. It stays in the Outliner.
	if (UBillboardComponent* Sprite = GetSpriteComponent())
	{
		Sprite->SetVisibility(false);
		Sprite->bHiddenInGame = true;
	}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
	bIsEditorOnlyActor = false;
	SetLockLocation(true);
	bCanPlayFromHere = 0;
#endif // WITH_EDITOR
}

ANWorldCollisionCacheActor* ANWorldCollisionCacheActor::Find(const UWorld* World)
{
	if (World == nullptr) return nullptr;

	// Mirrors FNWorldAssemblyUtils::GetCellActorFromWorld: a level scan rather than a TActorIterator, so that a level
	// loaded as an instance inside another does not hand back its host's cache.
	for (const ULevel* Level : World->GetLevels())
	{
		if (Level == nullptr || Level->IsInstancedLevel()) continue;

		for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt)
		{
			if (ActorIt->IsA<ANWorldCollisionCacheActor>())
			{
				return Cast<ANWorldCollisionCacheActor>(ActorIt->Get());
			}
		}
	}
	return nullptr;
}

#if WITH_EDITOR
ANWorldCollisionCacheActor* ANWorldCollisionCacheActor::FindOrCreate(UWorld* World)
{
	if (World == nullptr) return nullptr;

	if (ANWorldCollisionCacheActor* Existing = Find(World))
	{
		// Correct one saved before this actor was always-loaded. The flag is per-instance saved state, so changing the
		// class default leaves anything already on disk spatially loaded — and therefore invisible whenever the region
		// around the origin is not loaded, which reads as a level that was never baked.
		if (Existing->GetIsSpatiallyLoaded() && Existing->CanChangeIsSpatiallyLoadedFlag())
		{
			Existing->Modify();
			Existing->SetIsSpatiallyLoaded(false);
			Existing->MarkPackageDirty();

			UE_LOG(LogNexusWorldAssembly, Log,
				TEXT("World collision cache actor in '%s' was spatially loaded; marked always-loaded so the level's cache is found wherever the camera is."),
				*World->GetName());
		}
		return Existing;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.OverrideLevel = World->PersistentLevel;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags = RF_Transactional;

	return World->SpawnActor<ANWorldCollisionCacheActor>(ANWorldCollisionCacheActor::StaticClass(),
		FTransform::Identity, SpawnParameters);
}
#endif // WITH_EDITOR
