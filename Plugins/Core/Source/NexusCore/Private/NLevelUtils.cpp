// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NLevelUtils.h"

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
	const TArray<FName>& ActorIgnoreTags, const bool bIncludeEditorOnly, const bool bIncludeNonColliding, const bool bIncludeTransientActors)
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

			// Ignore Tags
			if (FNArrayUtils::ContainsAny(Actor->Tags, ActorIgnoreTags))
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			// Don't include transient actors
			if (!bIncludeTransientActors && Actor->HasAnyFlags(RF_Transient))
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			// Check Editor Only
			if (Actor->IsEditorOnly() && !bIncludeEditorOnly)
			{
				OutIgnoredActors.Add(Actor);
				continue;
			}

			const FBox ActorBox = Actor->GetComponentsBoundingBox(bIncludeNonColliding);
			if (ActorBox.IsValid)
			{
				OutBounds += ActorBox;
			}
		}
	}
}
