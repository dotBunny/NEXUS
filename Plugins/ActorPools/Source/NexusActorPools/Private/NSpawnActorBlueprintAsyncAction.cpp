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
	const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
	UNActorPoolSubsystem* ActorPoolSubsystem = UNActorPoolSubsystem::Get(World);
	if (!ActorPoolSubsystem)
	{
		UE_LOG(LogNexusActorPools, Error, TEXT("Unable to complete Spawn Actor Async as ActorPoolSubsystem is NULL"));
		SetReadyToDestroy();
		return;
	}
	
	UClass* ActorLoaded = ActorClass.Get();
	
	// Already has pool created
	if (ActorPoolSubsystem->HasActorPool(ActorLoaded))
	{
		OnHasPool(ActorPoolSubsystem->GetActorPool(ActorLoaded));
		return;
	}
	
	// Setup callback for when pool is added.
	OnCreatedPoolHandle = ActorPoolSubsystem->OnActorPoolAdded.AddUObject(this, &UNSpawnActorBlueprintAsyncAction::OnHasPool);
	ActorPoolSubsystem->CreateActorPool(ActorLoaded, ActorPoolSubsystem->GetDefaultSettings(ActorLoaded));
}

void UNSpawnActorBlueprintAsyncAction::OnHasPool(FNActorPool* ActorPool)
{
	// Not for me!
	if (ActorPool->GetTemplate() != ActorClass.Get()) return;
	
	// Unregister callback
	if (OnCreatedPoolHandle.IsValid())
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
		UNActorPoolSubsystem* ActorPoolSubsystem = UNActorPoolSubsystem::Get(World);
		ActorPoolSubsystem->OnActorPoolAdded.Remove(OnCreatedPoolHandle);
	}
	
	AActor* SpawnedActor = nullptr;
	if (const TSubclassOf<AActor> Class = ActorClass.Get(); Class && WorldContext.IsValid() && ActorPool != nullptr)
	{
		SpawnedActor = ActorPool->Spawn(Position, Rotation);
	}

	Completed.Broadcast(SpawnedActor);
	SetReadyToDestroy();
}
