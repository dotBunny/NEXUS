// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NLevelUtils.h"

#include "NActorUtils.h"
#include "NArrayUtils.h"
#include "Engine/ObjectLibrary.h"
#include "LevelInstance/LevelInstanceSubsystem.h"

ILevelInstanceInterface* FNLevelUtils::GetActorLevelInstance(const AActor* Actor)
{
	const UWorld* World = Actor->GetWorld();
	if (!World) return nullptr;

	if (const ULevelInstanceSubsystem* LevelInstanceSubsystem = World->GetSubsystem<ULevelInstanceSubsystem>())
	{
		return LevelInstanceSubsystem->GetOwningLevelInstance(Actor->GetLevel());
	}
	return nullptr;
}

TArray<FString> FNLevelUtils::GetAllMapNames(TArray<FString> SearchPaths)
{
	TArray<FString> ReturnArray;
	TArray<FAssetData> AssetDataArray;

	// Create our search library
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, true);

	// Load every search path into the library first. UObjectLibrary accumulates across LoadAssetDataFromPath calls.
	for (const FString& PathRoot : SearchPaths)
	{
		ObjectLibrary->LoadAssetDataFromPath(PathRoot);
	}

	// Harvest once. GetAssetDataList snapshots the whole accumulated library, so collecting inside the load loop
	// above would re-add every earlier path's maps on each subsequent iteration (one duplicate per extra path).
	ObjectLibrary->GetAssetDataList(AssetDataArray);
	ReturnArray.Reserve(AssetDataArray.Num());
	for (const FAssetData& Data : AssetDataArray)
	{
		ReturnArray.Add(Data.AssetName.ToString());
	}

	// Flag object to be destroyed
	ObjectLibrary->MarkAsGarbage();

	return ReturnArray;
}

void FNLevelUtils::DetermineLevelBounds(ULevel* InLevel, FBox& OutBounds, TArray<const AActor*>& OutIgnoredActors,
	const FNLevelBoundsFilter& Filter)
{
	// Ensure we have a valid level
	if (!IsValid(InLevel)) return;

	const int32 NumActors = InLevel->Actors.Num();

#if WITH_EDITOR
	FScopedSlowTask BoundsTask = FScopedSlowTask(NumActors, NSLOCTEXT("NexusCore", "Task_DetermineLevelBounds", "Determine Level Bounds"));
	BoundsTask.MakeDialog(false);
#endif // WITH_EDITOR

	// Initialize our empty box
	OutBounds = FBox(ForceInit);

	for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
	{
		const AActor* Actor = InLevel->Actors[ActorIndex];
#if WITH_EDITOR
		BoundsTask.EnterProgressFrame(1);
#endif // WITH_EDITOR

		// We do not want to have any bad actors at play
		if (!IsValid(Actor))
		{
			continue;
		}

		// Early check if we've ignored already
		if (OutIgnoredActors.Contains(Actor))
		{
			continue;
		}

		if (Actor && Actor->IsLevelBoundsRelevant())
		{

			// Terrain authoring apparatus is never geometry, at any setting — see FNActorUtils::IsTerrainAuthoringActor.
			// Unconditional rather than tied to bIncludeTerrain: that flag chooses whether terrain *geometry* counts,
			// and a modifier is not geometry under either answer.
			if (FNActorUtils::IsTerrainAuthoringActor(Actor))
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			// Ignore Tags
			if (FNArrayUtils::ContainsAny(Actor->Tags, Filter.ActorIgnoreTags))
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			const bool bIsTerrain = Filter.bIncludeTerrain && FNActorUtils::IsTerrainActor(Actor);

			// Don't include transient actors. Terrain is the deliberate exception: Mesh Partition represents an
			// authored terrain in the editor as transient APreviewSection actors spawned into the persistent level,
			// so the blanket skip would drop a level's entire floor from its bounds.
			if (!Filter.bIncludeTransientActors && Actor->HasAnyFlags(RF_Transient) && !bIsTerrain)
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			// Check Editor Only
			if (Actor->IsEditorOnly() && !Filter.bIncludeEditorOnly)
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			// Terrain goes through the placeholder-rejecting variant. A section that has not finished building reports
			// a near-zero box rather than an invalid one, and folding that in as a valid point drags the result out to
			// wherever the unbuilt component sits — which is what inflates bounds calculated moments after a terrain
			// edit, and why the same calculation is correct again once the level is reloaded.
			const FBox ActorBox = bIsTerrain
				? FNActorUtils::GetBuiltComponentsBoundingBox(Actor, Filter.bIncludeNonColliding)
				: Actor->GetComponentsBoundingBox(Filter.bIncludeNonColliding);
			if (ActorBox.IsValid)
			{
				OutBounds += ActorBox;
			}
		}
	}
}
