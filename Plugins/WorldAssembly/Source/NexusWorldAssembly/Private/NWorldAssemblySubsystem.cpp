// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblySubsystem.h"

#include "NMultiplayerUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyRelay.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

void UNWorldAssemblySubsystem::Generate(FNAssemblyOperationSettings& Settings)
{
	UNAssemblyOperation* Operation = UNAssemblyOperation::CreateInstance(
		FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(GetWorld()->GetCurrentLevel()), Settings);

	StartOperation(Operation);
}

bool UNWorldAssemblySubsystem::IsReady()
{
	// Server always has stuff replicated
	if (FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
	{
		return KnownOperations.IsEmpty();
	}
	
	// Client hasn't spawned the goodness yet
	if (LocalRelay == nullptr) return false;
	
	// Client properly checking
	return LocalRelay->IsReady();
}

void UNWorldAssemblySubsystem::Tick(float DeltaTime)
{
	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		KnownOperations[i]->Tick();
	}
}

bool UNWorldAssemblySubsystem::IsTickable() const
{
	if (KnownOperations.Num() > 0) return true;
	return false;
}

void UNWorldAssemblySubsystem::StartOperation(UNAssemblyOperation* Operation)
{
	if (Operation == nullptr) return;
	
	KnownOperations.AddUnique(Operation);
	OnOperationStarted.Broadcast();
	
	Operation->StartBuild(this);

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
	LocalRelay = nullptr;

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
