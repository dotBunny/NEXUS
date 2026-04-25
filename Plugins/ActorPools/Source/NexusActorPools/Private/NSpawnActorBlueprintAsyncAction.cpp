// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSpawnActorBlueprintAsyncAction.h"
#include "NActorPoolSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Macros/NWorldMacros.h"

UNSpawnActorBlueprintAsyncAction* UNSpawnActorBlueprintAsyncAction::SpawnActorAsync(UObject* WorldContextObject, TSoftClassPtr<AActor> ActorClass, FVector Position, FRotator Rotation)
{
	UNSpawnActorBlueprintAsyncAction* Action = NewObject<UNSpawnActorBlueprintAsyncAction>();
	Action->WorldContext = WorldContextObject;
	Action->ActorClass = ActorClass;
	Action->Position = Position;
	Action->Rotation = Rotation;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UNSpawnActorBlueprintAsyncAction::Activate()
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
		FStreamableDelegate::CreateUObject(this, &UNSpawnActorBlueprintAsyncAction::OnLoaded)
	);
}

void UNSpawnActorBlueprintAsyncAction::OnLoaded()
{
	//const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
	//UClass* Actor = ActorClass.Get();
	
	// ActorPoolSubsystem->CreateActorPool(Actor, ActorPoolSubsystem->GetDefaultSettings(Actor));
	

	OnHasPool();
}

void UNSpawnActorBlueprintAsyncAction::OnHasPool()
{
	AActor* SpawnedActor = nullptr;
	if (const TSubclassOf<AActor> Class = ActorClass.Get(); Class && WorldContext.IsValid())
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
		if (UNActorPoolSubsystem* ActorPoolSubsystem = UNActorPoolSubsystem::Get(World))
		{
			ActorPoolSubsystem->SpawnActor(Class, Position, Rotation, SpawnedActor);
		}
	}
	Completed.Broadcast(SpawnedActor);
	SetReadyToDestroy();
}
