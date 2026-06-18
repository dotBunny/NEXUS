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

void UNGetActorBlueprintAsyncAction::BeginDestroy()
{
	if (OnCreatedPoolHandle.IsValid())
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
		UNActorPoolSubsystem* ActorPoolSubsystem = UNActorPoolSubsystem::Get(World);
		if (ActorPoolSubsystem)
		{
			ActorPoolSubsystem->OnActorPoolAdded.Remove(OnCreatedPoolHandle);
		}
	}

	// Cancel any load still in flight on teardown (e.g. PIE-end / level transition) rather than leaving the
	// streamable request to run to completion and the handle to be released only at GC.
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
	Super::BeginDestroy();
}

void UNGetActorBlueprintAsyncAction::OnLoaded()
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
	if (ActorLoaded == nullptr)
	{
		// The soft class resolved to null (e.g. the referenced asset was deleted/renamed). CreateActorPool
		// would reject a null class and never broadcast OnActorPoolAdded, leaving this latent node hung and
		// the action object alive until GC. Complete with null and tear down instead.
		UE_LOG(LogNexusActorPools, Error, TEXT("Unable to complete Get Actor Async as the soft class failed to load (%s)."), *ActorClass.ToString());
		Completed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	// Already has pool created
	if (ActorPoolSubsystem->HasActorPool(ActorLoaded))
	{
		OnHasPool(ActorPoolSubsystem->GetActorPool(ActorLoaded));
		return;
	}
	
	// Setup callback for when pool is added.
	OnCreatedPoolHandle = ActorPoolSubsystem->OnActorPoolAdded.AddUObject(this, &UNGetActorBlueprintAsyncAction::OnHasPool);
	ActorPoolSubsystem->CreateActorPool(ActorLoaded, ActorPoolSubsystem->GetDefaultSettings(ActorLoaded));
}

void UNGetActorBlueprintAsyncAction::OnHasPool(FNActorPool* ActorPool)
{
	// Not for me!
	if (ActorPool == nullptr || ActorPool->GetTemplate() != ActorClass.Get()) return;
	
	// Unregister callback
	if (OnCreatedPoolHandle.IsValid())
	{
		const UWorld* World = N_GET_WORLD_FROM_CONTEXT(WorldContext.Get());
		// Guard the subsystem lookup (the world/context may have gone away by now) — matches the
		// SpawnActorAsync twin, which already null-checks here.
		if (UNActorPoolSubsystem* ActorPoolSubsystem = UNActorPoolSubsystem::Get(World))
		{
			ActorPoolSubsystem->OnActorPoolAdded.Remove(OnCreatedPoolHandle);
		}
		OnCreatedPoolHandle.Reset();
	}
	
	AActor* SpawnedActor = nullptr;
	if (const TSubclassOf<AActor> Class = ActorClass.Get(); Class && WorldContext.IsValid() && ActorPool != nullptr)
	{
		SpawnedActor = ActorPool->Get();
	}

	Completed.Broadcast(SpawnedActor);
	SetReadyToDestroy();
}
