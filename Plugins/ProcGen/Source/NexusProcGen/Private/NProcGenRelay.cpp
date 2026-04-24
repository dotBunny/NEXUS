// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenRelay.h"

#include "NMultiplayerUtils.h"
#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "NProcGenSubsystem.h"
#include "Cell/NCellLevelInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ANProcGenRelay::ANProcGenRelay()
{
	bReplicates = true;
	bOnlyRelevantToOwner = true;
	bNetLoadOnClient = false;
	SetReplicatingMovement(false);
}

bool ANProcGenRelay::IsReady()
{
	return KnownOperations.Num() == 0 && HasNearbyCellLevelInstances();
}

bool ANProcGenRelay::HasNearbyCellLevelInstances()
{
	// Early out the best we can
	if (bHasNearbyCellLevelInstances) return true;
	if (CachedNearbyCellLevelInstances.Num() == 0)  return false;
	
	// If we haven't got them lets reupdate and respond accordingly
	bHasNearbyCellLevelInstances = FNProcGenRegistry::HasCellLevelInstances(CachedNearbyCellLevelInstances);
	return bHasNearbyCellLevelInstances;
}

void ANProcGenRelay::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC != nullptr && OwningPC->IsLocalController())
	{
		if (UNProcGenSubsystem* Subsystem = UNProcGenSubsystem::Get(GetWorld()))
		{
			Subsystem->RegisterLocalRelay(this);
		}
		// Initial check
		UpdateNearbyCells();
	}
}

void ANProcGenRelay::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC != nullptr && OwningPC->IsLocalController())
	{
		if (UNProcGenSubsystem* Subsystem = UNProcGenSubsystem::Get(GetWorld()))
		{
			Subsystem->UnregisterLocalRelay(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANProcGenRelay::UpdateNearbyCells(bool bIsLevelLoaded)
{
	bHasNearbyCellLevelInstances = false;
	CachedNearbyCellLevelInstances.Empty();
	
	// Setup initial query
	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	
	FVector Location  = OwningPC->GetSpawnLocation();
	APawn* SafePawn = OwningPC->GetPawnOrSpectator();
	if (SafePawn!= nullptr)
	{
		Location = SafePawn->GetActorLocation();
	}
	UE_LOG(LogNexusProcGen, Log, TEXT("Request nearby(%s) ANCellLevelInstances list."), *Location.ToString());
	Server_RequestNearbyCells(Location, 0, bIsLevelLoaded);
}

void ANProcGenRelay::Client_OperationFinished_Implementation(const uint32 OperationTicket)
{
	UE_LOG(LogNexusProcGen, Log, TEXT("Received finished notification for operation(%i)."), OperationTicket);	
	KnownOperations.RemoveSwap(OperationTicket);
	
	UpdateNearbyCells(true);
}

void ANProcGenRelay::Client_OperationStarted_Implementation(const uint32 OperationTicket)
{
	UE_LOG(LogNexusProcGen, Log, TEXT("Received starting notification for operation(%i)."), OperationTicket);
	KnownOperations.AddUnique(OperationTicket);
}

void ANProcGenRelay::Client_OperationDestroyed_Implementation(const uint32 OperationTicket)
{
	UE_LOG(LogNexusProcGen, Log, TEXT("Received destruction notification for operation(%i)."), OperationTicket);
	KnownOperations.RemoveSwap(OperationTicket);
}


void ANProcGenRelay::Server_RequestNearbyCells_Implementation(const FVector Location, const uint32 OperationTicket, const bool bIsLevelLoaded)
{
	// Find our instances
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	
	// The server often will get this call before the levels have streamed, thus we need to flag that the search is not looking to see if they are
	// loaded when we send it back to the player, but instead just that they are there in place.
	const TArray<ANCellLevelInstance*> Instances = FNProcGenRegistry::GetCellLevelInstancesInRange(Location, Settings->NetworkNearbyRange, false, OperationTicket);

	// Create our return array
	TArray<FNCellLevelInstanceLocator> Results;
	Results.Reserve(Instances.Num());
	for (ANCellLevelInstance* Instance : Instances)
	{
		FNCellLevelInstanceLocator& Locator = Results.AddDefaulted_GetRef();
		Locator.OperationTicket = Instance->GetOperationTicket();
		Locator.LevelInstanceSpawnGuid = Instance->GetLevelInstanceSpawnGuid();
		Locator.Location = Instance->GetActorLocation();
	}

	// Send it!
	Client_ReceiveNearbyCells(Results);
}

void ANProcGenRelay::Client_ReceiveNearbyCells_Implementation(const TArray<FNCellLevelInstanceLocator>& Results)
{
	UE_LOG(LogNexusProcGen, Log, TEXT("Received nearby list(%i) of ANCellLevelInstances for player."), Results.Num());
	CachedNearbyCellLevelInstances = Results; // Copy
}
