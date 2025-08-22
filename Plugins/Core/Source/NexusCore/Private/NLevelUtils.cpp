// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NLevelUtils.h"
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
