// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"

#include "NMultiplayerUtils.h"
#include "NProcGenOperation.h"
#include "NProcGenRegistry.h"
#include "NProcGenRelay.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

void UNProcGenSubsystem::Generate(FNProcGenOperationSettings& Settings)
{
	UNProcGenOperation* Operation = UNProcGenOperation::CreateInstance(
		FNProcGenRegistry::GetOrganComponentsFromLevel(GetWorld()->GetCurrentLevel()), Settings);

	StartOperation(Operation);
}

bool UNProcGenSubsystem::IsReady()
{
	// Server always has stuff replicated
	if (FNMultiplayerUtils::HasWorldAuthority(GetWorld()))
	{
		return KnownOperations.Num() == 0;
	}
	
	// Client hasn't spawned the goodness yet
	if (LocalRelay == nullptr) return false;
	
	// Client properly checking
	return LocalRelay->IsReady();
}

void UNProcGenSubsystem::Tick(float DeltaTime)
{
	for (const auto Operation : KnownOperations)
	{
		Operation->Tick();
	}
}

bool UNProcGenSubsystem::IsTickable() const
{
	if (KnownOperations.Num() > 0) return true;
	return false;
}

void UNProcGenSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);

	// Notify Relays
	for (const auto Pair : RelayMap)
	{
		Pair.Value->Client_OperationStarted(Operation->GetTicket());
	}
}

void UNProcGenSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext)
{
	KnownOperations.Remove(Operation);

	// Notify Relays
	for (const auto Pair : RelayMap)
	{
		Pair.Value->Client_OperationFinished(Operation->GetTicket());
	}
}

void UNProcGenSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
	
	// Notify Relays
	for (const auto Pair : RelayMap)
	{
		Pair.Value->Client_OperationDestroyed(Operation->GetTicket());
	}	
}

void UNProcGenSubsystem::RegisterLocalRelay(ANProcGenRelay* InRelay)
{
	LocalRelay = InRelay;
}

void UNProcGenSubsystem::UnregisterLocalRelay(const ANProcGenRelay* InRelay)
{
	if (LocalRelay == InRelay)
	{
		LocalRelay = nullptr;
	}
}

void UNProcGenSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	// Only do this on the server
	if (FNMultiplayerUtils::HasWorldAuthority(InWorld))
	{
		OnLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UNProcGenSubsystem::OnPostLogin);
		OnLogoutHandle = FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &UNProcGenSubsystem::OnLogout);
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

void UNProcGenSubsystem::Deinitialize()
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

void UNProcGenSubsystem::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (GameMode == nullptr || GameMode->GetWorld() != GetWorld()) return;
	SpawnRelay(NewPlayer);
}

void UNProcGenSubsystem::OnLogout(AGameModeBase* GameMode, AController* Exiting)
{
	if (GameMode == nullptr || GameMode->GetWorld() != GetWorld()) return;
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		DestroyRelay(PC);
	}
}

void UNProcGenSubsystem::SpawnRelay(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return;
	if (RelayMap.Contains(PlayerController)) return;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = PlayerController;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANProcGenRelay* NewRelay = GetWorld()->SpawnActor<ANProcGenRelay>(ANProcGenRelay::StaticClass(), SpawnInfo);
	if (NewRelay == nullptr)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to spawn ANProcGenRelay for PlayerController(%s)."), *PlayerController->GetName());
		return;
	}

	RelayMap.Add(PlayerController, NewRelay);
}

void UNProcGenSubsystem::DestroyRelay(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return;
	TObjectPtr<ANProcGenRelay> Existing;
	if (RelayMap.RemoveAndCopyValue(PlayerController, Existing) && Existing != nullptr)
	{
		Existing->Destroy();
	}
}
