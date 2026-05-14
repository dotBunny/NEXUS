// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyRelay.h"

#include "NMultiplayerUtils.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblySubsystem.h"
#include "Cell/NCellLevelInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ANWorldAssemblyRelay::ANWorldAssemblyRelay()
{
	bReplicates = true;
	bOnlyRelevantToOwner = true;
	bNetLoadOnClient = false;
	SetReplicatingMovement(false);
}

bool ANWorldAssemblyRelay::IsReady()
{
	return KnownOperations.Num() == 0 && HasNearbyCellLevelInstances();
}

bool ANWorldAssemblyRelay::HasNearbyCellLevelInstances()
{
	// Early out the best we can
	if (bHasNearbyCellLevelInstances) return true;
	if (CachedNearbyCellLevelInstances.Num() == 0)  return false;
	
	// If we haven't got them lets reupdate and respond accordingly
	bHasNearbyCellLevelInstances = FNWorldAssemblyRegistry::HasCellLevelInstances(CachedNearbyCellLevelInstances);
	return bHasNearbyCellLevelInstances;
}

void ANWorldAssemblyRelay::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC != nullptr && OwningPC->IsLocalController())
	{
		if (UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(GetWorld()))
		{
			Subsystem->RegisterLocalRelay(this);
		}
		// Initial check
		UpdateNearbyCells();
	}
}

void ANWorldAssemblyRelay::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC != nullptr && OwningPC->IsLocalController())
	{
		if (UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(GetWorld()))
		{
			Subsystem->UnregisterLocalRelay(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANWorldAssemblyRelay::UpdateNearbyCells(bool bIsLevelLoaded)
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
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Request nearby(%s) ANCellLevelInstances list."), *Location.ToString());
	Server_RequestNearbyCells(Location, 0, bIsLevelLoaded);
}

void ANWorldAssemblyRelay::Client_OperationFinished_Implementation(const uint32 OperationTicket)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received finished notification for operation(%i)."), OperationTicket);	
	KnownOperations.RemoveSwap(OperationTicket);
	
	UpdateNearbyCells(true);
}

void ANWorldAssemblyRelay::Client_OperationStarted_Implementation(const uint32 OperationTicket)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received starting notification for operation(%i)."), OperationTicket);
	KnownOperations.AddUnique(OperationTicket);
}

void ANWorldAssemblyRelay::Client_OperationDestroyed_Implementation(const uint32 OperationTicket)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received destruction notification for operation(%i)."), OperationTicket);
	KnownOperations.RemoveSwap(OperationTicket);
}


void ANWorldAssemblyRelay::Server_RequestNearbyCells_Implementation(const FVector Location, const uint32 OperationTicket, const bool bIsLevelLoaded)
{
	// Find our instances
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	
	// The server often will get this call before the levels have streamed, thus we need to flag that the search is not looking to see if they are
	// loaded when we send it back to the player, but instead just that they are there in place.
	const TArray<ANCellLevelInstance*> Instances = FNWorldAssemblyRegistry::GetCellLevelInstancesInRange(Location, Settings->NetworkNearbyRange, false, OperationTicket);

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

void ANWorldAssemblyRelay::Client_ReceiveNearbyCells_Implementation(const TArray<FNCellLevelInstanceLocator>& Results)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received nearby list(%i) of ANCellLevelInstances for player."), Results.Num());
	CachedNearbyCellLevelInstances = Results; // Copy
}
