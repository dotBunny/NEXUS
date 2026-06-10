// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblySubsystem.h"


#include "EngineUtils.h"
#include "NMultiplayerUtils.h"
#include "NWorldAssemblyContextCache.h"
#include "NWorldUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyRelay.h"
#include "Cell/NCellProxy.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

namespace NEXUS::WorldAssembly::ConsoleCommands
{
	static FAutoConsoleCommand WorldAssemblyRegenerate(TEXT("N.WorldAssembly.Regenerate"),
		TEXT("Regenerates all Organs in a given world, clearing the previous generation."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []
			{
				UWorld* World = FNWorldUtils::GetGameWorld();
				if (World == nullptr)
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to regenerate the world, as the world was NULL."));
					return;
				}
				
				if (FNMultiplayerUtils::HasWorldAuthority(World))
				{
					UNWorldAssemblySubsystem* System = UNWorldAssemblySubsystem::Get(World);
					System->Clear();
					
					FNAssemblyOperationSettings Settings = FNAssemblyOperationSettings::GetDefaultSettings();
					System->Generate(Settings);
				}
				else
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to regenerate the world as you do not have authority."));
				}
			});
		}));
	static FAutoConsoleCommand WorldAssemblyClear(TEXT("N.WorldAssembly.Clear"),
		TEXT("Clears the previous generation and any registered cleanup actors."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []
			{
				UWorld* World = FNWorldUtils::GetGameWorld();
				if (World == nullptr)
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to clear the world, as the world was NULL."));
					return;
				}
				
				if (FNMultiplayerUtils::HasWorldAuthority(World))
				{
					UNWorldAssemblySubsystem* System = UNWorldAssemblySubsystem::Get(World);
					System->Clear();
				}
				else
				{
					UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to clear the world as you do not have authority."));
				}
			});
		}));
}

void UNWorldAssemblySubsystem::Generate(FNAssemblyOperationSettings& Settings)
{
	UNAssemblyOperation* Operation = UNAssemblyOperation::CreateInstance(
		FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(GetWorld()->GetCurrentLevel()), Settings);

	StartOperation(Operation);
}

void UNWorldAssemblySubsystem::Clear()
{
	UWorld* World = GetWorld();
	
	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		if (KnownOperations[i]->IsRunning())
		{
			if (FNAssemblyTaskGraph* Graph = KnownOperations[i]->GetTaskGraph())
			{
				Graph->Cancel();
			}
		}
	}

#if WITH_EDITOR
	// Bulk clears can tear down streamed sublevel actors the user may have selected; drop the entire
	// selection so the typed-element registry does not assert on a stale handle next mouse-move.
	if (GIsEditor && GEditor != nullptr)
	{
		GEditor->SelectNone(false, true, false);
	}
#endif // WITH_EDITOR

	for (TActorIterator<ANCellProxy> It(World, ANCellProxy::StaticClass()); It; ++It)
	{
		ANCellProxy* Proxy = *It;
		Proxy->DestroyLevelInstance(true, true);
		Proxy->Destroy();
	}
	
	// Handle our track for cleanup
	for (TWeakObjectPtr Actor : TrackedActorsForCleanup)
	{
		if (Actor.IsValid())
		{
			Actor.Get()->Destroy(true, false);
		}
	}
	TrackedActorsForCleanup.Empty();
	
	// Allow folks to subscribe for this event
	OnCleared.Broadcast();
}

bool UNWorldAssemblySubsystem::IsReady(const bool bWaitOnStreaming)
{
	const UWorld* World = GetWorld();
	if (bWaitOnStreaming && FNWorldUtils::IsStreaming(World))
	{
		return false;
	}
	
	// Server always has stuff replicated
	if (FNMultiplayerUtils::HasWorldAuthority(World))
	{
		return KnownOperations.IsEmpty();
	}
	
	// Client hasn't spawned the goodness yet
	if (LocalRelay == nullptr) return false;
	
	// Client properly checking
	return LocalRelay->IsReady();
}

FIntVector2 UNWorldAssemblySubsystem::GetRemainingStatus()
{
	if (LocalRelay == nullptr)
	{
		return FIntVector2::ZeroValue;
	}
	
	return LocalRelay->GetRemainingStatus();
}

void UNWorldAssemblySubsystem::RegisterActorForCleanup(AActor* Actor)
{
	TrackedActorsForCleanup.AddUnique(Actor);
}

void UNWorldAssemblySubsystem::UnregisterActorForCleanup(AActor* Actor)
{
	TrackedActorsForCleanup.Remove(Actor);
}

void UNWorldAssemblySubsystem::Tick(float DeltaTime)
{
	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		KnownOperations[i]->Tick();
	}
	
	// If we have anything queued for generation, lets get it going
	if (QueuedOrgansForAssembly.Num() > 0)
	{
		FNAssemblyOperationSettings Settings = FNAssemblyOperationSettings::GetDefaultSettings();
		Settings.DisplayName = FText::FromString("QueuedOrganAssembly");
		UNAssemblyOperation* InstanceOperation = UNAssemblyOperation::CreateInstance(QueuedOrgansForAssembly, Settings);

		StartOperation(InstanceOperation);
		
		// We issue the generation and then clear the queue.
		QueuedOrgansForAssembly.Empty();
	}
}

bool UNWorldAssemblySubsystem::IsTickable() const
{
	if (KnownOperations.Num() > 0 || QueuedOrgansForAssembly.Num() > 0) return true;
	return false;
}

void UNWorldAssemblySubsystem::StartOperation(UNAssemblyOperation* Operation)
{
	if (Operation == nullptr) return;
	
	KnownOperations.AddUnique(Operation);
	OnOperationStarted.Broadcast();
	CachedOperationTickets.Add(Operation->GetTicket());
	
	Operation->StartBuild(this, this);

	// Snapshot to guard against reentrant mutation of RelayMap during the broadcast.
	TArray<TObjectPtr<ANWorldAssemblyRelay>> Relays;
	RelayMap.GenerateValueArray(Relays);
	for (ANWorldAssemblyRelay* Relay : Relays)
	{
		if (IsValid(Relay))
		{
			Relay->Client_OperationStarted(Operation->GetTicket());
		}
	}
}

void UNWorldAssemblySubsystem::OnOperationFinished(UNAssemblyOperation* Operation, TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext)
{
	const int RemoveCount = KnownOperations.Remove(Operation);
	if (RemoveCount > 0 && KnownOperations.IsEmpty())
	{
		OnOperationsCompleted.Broadcast();
	}

	// Snapshot to guard against reentrant mutation of RelayMap during the broadcast.
	TArray<TObjectPtr<ANWorldAssemblyRelay>> Relays;
	RelayMap.GenerateValueArray(Relays);
	for (ANWorldAssemblyRelay* Relay : Relays)
	{
		if (IsValid(Relay))
		{
			Relay->Client_OperationFinished(Operation->GetTicket());
		}
	}
}

void UNWorldAssemblySubsystem::OnOperationDestroyed(UNAssemblyOperation* Operation)
{
	const int RemoveCount = KnownOperations.Remove(Operation);
	if (RemoveCount > 0 && KnownOperations.IsEmpty())
	{
		OnOperationsCompleted.Broadcast();
	}

	// Snapshot to guard against reentrant mutation of RelayMap during the broadcast.
	TArray<TObjectPtr<ANWorldAssemblyRelay>> Relays;
	RelayMap.GenerateValueArray(Relays);
	for (ANWorldAssemblyRelay* Relay : Relays)
	{
		if (IsValid(Relay))
		{
			Relay->Client_OperationDestroyed(Operation->GetTicket());
		}
	}
}

void UNWorldAssemblySubsystem::RegisterLocalRelay(ANWorldAssemblyRelay* InRelay)
{
	LocalRelay = InRelay;
}

void UNWorldAssemblySubsystem::UnregisterLocalRelay(const ANWorldAssemblyRelay* InRelay)
{
	if (LocalRelay == InRelay)
	{
		LocalRelay = nullptr;
	}
}

void UNWorldAssemblySubsystem::RegisterOrganForAssembly(TObjectPtr<UNOrganComponent> Organ)
{
	QueuedOrgansForAssembly.AddUnique(Organ);
}

void UNWorldAssemblySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	// Only do this on the server
	if (FNMultiplayerUtils::HasWorldAuthority(InWorld))
	{
		OnLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UNWorldAssemblySubsystem::OnPostLogin);
		OnLogoutHandle = FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &UNWorldAssemblySubsystem::OnLogout);
		for (FConstPlayerControllerIterator It = InWorld.GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				SpawnRelay(PC);
			}
		}
	}

	Super::OnWorldBeginPlay(InWorld);
}

void UNWorldAssemblySubsystem::Deinitialize()
{
	// Clear cached persistent operation data
	if (CachedOperationTickets.Num() > 0)
	{
		FNWorldAssemblyContextCache::ClearContext(CachedOperationTickets);
		CachedOperationTickets.Empty();
	}
	
	if (OnLoginHandle.IsValid())
	{
		FGameModeEvents::GameModePostLoginEvent.Remove(OnLoginHandle);
		OnLoginHandle.Reset();
	}
	if (OnLogoutHandle.IsValid())
	{
		FGameModeEvents::GameModeLogoutEvent.Remove(OnLogoutHandle);
		OnLogoutHandle.Reset();
	}
	RelayMap.Reset();
	TrackedActorsForCleanup.Empty();
	
	LocalRelay = nullptr;
	
	// Stop all known operations
	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		UNAssemblyOperation* Operation = KnownOperations[i];
		if (Operation->IsRunning())
		{
			Operation->Cancel();
		}
		else
		{
			Operation->TearDownOperation();
		}
	}

	Super::Deinitialize();
}

void UNWorldAssemblySubsystem::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (GameMode == nullptr || GameMode->GetWorld() != GetWorld()) return;
	SpawnRelay(NewPlayer);
}

void UNWorldAssemblySubsystem::OnLogout(AGameModeBase* GameMode, AController* Exiting)
{
	if (GameMode == nullptr || GameMode->GetWorld() != GetWorld()) return;
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		DestroyRelay(PC);
	}
}

void UNWorldAssemblySubsystem::SpawnRelay(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return;
	if (RelayMap.Contains(PlayerController)) return;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = PlayerController;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANWorldAssemblyRelay* NewRelay = GetWorld()->SpawnActor<ANWorldAssemblyRelay>(ANWorldAssemblyRelay::StaticClass(), SpawnInfo);
	if (NewRelay == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to spawn ANWorldAssemblyRelay for PlayerController(%s)."), *PlayerController->GetName());
		return;
	}

	RelayMap.Add(PlayerController, NewRelay);
}

void UNWorldAssemblySubsystem::DestroyRelay(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return;
	TObjectPtr<ANWorldAssemblyRelay> Existing;
	if (RelayMap.RemoveAndCopyValue(PlayerController, Existing) && Existing != nullptr)
	{
		Existing->Destroy();
	}
}
