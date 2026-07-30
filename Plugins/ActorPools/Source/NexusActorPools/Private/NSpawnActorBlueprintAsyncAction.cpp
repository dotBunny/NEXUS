// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSpawnActorBlueprintAsyncAction.h"
#include "NActorPoolSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"

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

void UNSpawnActorBlueprintAsyncAction::BeginDestroy()
{
	if (OnCreatedPoolHandle.IsValid())
	{
		if (UNActorPoolSubsystem* ActorPoolSubsystem = ResolveActorPoolSubsystem())
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

void UNSpawnActorBlueprintAsyncAction::OnLoaded()
{
	UNActorPoolSubsystem* ActorPoolSubsystem = ResolveActorPoolSubsystem();
	if (!ActorPoolSubsystem)
	{
		// The world context expired (or carries no pool subsystem) while the class streamed in — the level
		// changed mid-load. Complete with null so the latent node releases instead of hanging, then tear down.
		UE_LOG(LogNexusActorPools, Warning, TEXT("Spawn Actor Async completed after its world context expired; completing with null."));
		Completed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	UClass* ActorLoaded = ActorClass.Get();
	if (ActorLoaded == nullptr)
	{
		// The soft class resolved to null (e.g. the referenced asset was deleted/renamed). CreateActorPool
		// would reject a null class and never broadcast OnActorPoolAdded, leaving this latent node hung and
		// the action object alive until GC. Complete with null and tear down instead.
		UE_LOG(LogNexusActorPools, Error, TEXT("Unable to complete Spawn Actor Async as the soft class failed to load (%s)."), *ActorClass.ToString());
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
	OnCreatedPoolHandle = ActorPoolSubsystem->OnActorPoolAdded.AddUObject(this, &UNSpawnActorBlueprintAsyncAction::OnHasPool);
	ActorPoolSubsystem->CreateActorPool(ActorLoaded, ActorPoolSubsystem->GetDefaultSettings(ActorLoaded));
}

void UNSpawnActorBlueprintAsyncAction::OnHasPool(FNActorPool* ActorPool)
{
	// Not for me!
	if (ActorPool == nullptr || ActorPool->GetTemplate() != ActorClass.Get()) return;

	// Unregister callback
	if (OnCreatedPoolHandle.IsValid())
	{
		if (UNActorPoolSubsystem* ActorPoolSubsystem = ResolveActorPoolSubsystem())
		{
			ActorPoolSubsystem->OnActorPoolAdded.Remove(OnCreatedPoolHandle);
		}
		OnCreatedPoolHandle.Reset();
	}

	AActor* SpawnedActor = nullptr;
	if (const TSubclassOf<AActor> Class = ActorClass.Get(); Class && WorldContext.IsValid() && ActorPool != nullptr)
	{
		SpawnedActor = ActorPool->Spawn(Position, Rotation);
	}

	Completed.Broadcast(SpawnedActor);
	SetReadyToDestroy();
}

UNActorPoolSubsystem* UNSpawnActorBlueprintAsyncAction::ResolveActorPoolSubsystem() const
{
	// WorldContext is a weak pointer that legitimately expires when the level changes while the class is
	// still streaming in — the exact scenario async loading exists for. Resolve defensively (ReturnNull in
	// every build) instead of via N_GET_WORLD_FROM_CONTEXT, whose shipping arm dereferences a null context,
	// and never feed a null world into Get(), which dereferences it unconditionally.
	if (UObject* Context = WorldContext.Get())
	{
		if (const UWorld* World = GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::ReturnNull))
		{
			return UNActorPoolSubsystem::Get(World);
		}
	}
	return nullptr;
}
