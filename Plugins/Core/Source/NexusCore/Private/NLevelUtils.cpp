// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NLevelUtils.h"

#include "NArrayUtils.h"
#include "Engine/ObjectLibrary.h"
#include "LevelInstance/LevelInstanceSubsystem.h"

ILevelInstanceInterface* FNLevelUtils::GetActorLevelInstance(const AActor* Actor)
{
	if (const ULevelInstanceSubsystem* LevelInstanceSubsystem = Actor->GetWorld()->GetSubsystem<ULevelInstanceSubsystem>())
	{
		return LevelInstanceSubsystem->GetOwningLevelInstance(Actor->GetLevel());
	}
	return nullptr;
}

TArray<FString> FNLevelUtils::GetAllMapNames(TArray<FString> SearchPaths)
{
	TArray<FString> ReturnArray = TArray<FString>();
	TArray<FAssetData> AssetDataArray;

	// Create our search library
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, true);
	
	for (FString& PathRoot : SearchPaths)
	{
		ObjectLibrary->LoadAssetDataFromPath(PathRoot);
		ObjectLibrary->GetAssetDataList(AssetDataArray);

		const int32 Count = AssetDataArray.Num();
		for (int32 i = 0; i < Count; ++i)
		{
			ReturnArray.Add(AssetDataArray[i].AssetName.ToString());
		}
	}

	// Flag object to be destroyed
	ObjectLibrary->ConditionalBeginDestroy();

	return MoveTemp(ReturnArray);
}

void FNLevelUtils::DetermineLevelBounds(ULevel* InLevel, FBox& OutBounds, TArray<const AActor*>& OutIgnoredActors,
	const TArray<FName>& ActorIgnoreTags, const bool bIncludeEditorOnly, const bool bIncludeNonColliding)
{
	const int32 NumActors = InLevel->Actors.Num();
	
#if WITH_EDITOR			
	FScopedSlowTask BoundsTask = FScopedSlowTask(NumActors, NSLOCTEXT("NexusCore", "Task_DetermineLevelBounds", "Determine Level Bounds"));
	BoundsTask.MakeDialog(false);
#endif // WITH_EDITOR	
	
	for (int32 ActorIndex = 0; ActorIndex < NumActors; ++ActorIndex)
	{
		const AActor* Actor = InLevel->Actors[ActorIndex];
#if WITH_EDITOR			
		BoundsTask.EnterProgressFrame(1);
#endif // WITH_EDITOR

		if (Actor && Actor->IsLevelBoundsRelevant())
		{
			// Ignore Tags
			if (FNArrayUtils::ContainsAny(Actor->Tags, ActorIgnoreTags))
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
