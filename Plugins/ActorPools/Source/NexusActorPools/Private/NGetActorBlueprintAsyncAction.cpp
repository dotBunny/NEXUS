// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGetActorBlueprintAsyncAction.h"
#include "NActorPoolSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Macros/NWorldMacros.h"

UNGetActorBlueprintAsyncAction* UNGetActorBlueprintAsyncAction::GetActorAsync(UObject* WorldContextObject, TSoftClassPtr<AActor> ActorClass)
{
	UNGetActorBlueprintAsyncAction* Action = NewObject<UNGetActorBlueprintAsyncAction>();
	Action->WorldContext = WorldContextObject;
	Action->ActorClass = ActorClass;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UNGetActorBlueprintAsyncAction::Activate()
{
	if (ActorClass.IsNull())
	{
		Completed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	StreamingHandle = StreamableManager.RequestAsyncLoad(
		ActorClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UNGetActorBlueprintAsyncAction::OnLoaded)
	);
}

void UNGetActorBlueprintAsyncAction::OnLoaded()
{
	OnHasPool();
}

void UNGetActorBlueprintAsyncAction::OnHasPool()
{
	AActor* SpawnedActor = nullptr;
	if (const TSubclassOf<AActor> Class = ActorClass.Get(); Class && WorldContext.IsValid())
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
		if (UNActorPoolSubsystem* ActorPoolSubsystem = UNActorPoolSubsystem::Get(World))
		{
			ActorPoolSubsystem->GetActor(Class, SpawnedActor);
		}
	}
	Completed.Broadcast(SpawnedActor);
	SetReadyToDestroy();
}
