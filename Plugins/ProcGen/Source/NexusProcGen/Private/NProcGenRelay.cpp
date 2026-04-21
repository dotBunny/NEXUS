// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenRelay.h"

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

bool ANProcGenRelay::HasInitialCellLevelInstances()
{
	// Early out the best we can
	if (bHasInitialCellLevelInstances) return true;
	if (CachedInitialCellLevelInstances.Num() == 0)  return false;
	
	// If we haven't got them lets reupdate and respond accordingly
	bHasInitialCellLevelInstances = FNProcGenRegistry::HasCellLevelInstances(CachedInitialCellLevelInstances);
	return bHasInitialCellLevelInstances;
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
		
		// Setup initial query
		OnInitialQueryHandle = OnQueryComplete.AddUObject(this, &ANProcGenRelay::OnInitialQueryComplete);
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

void ANProcGenRelay::OnInitialQueryComplete(const TArray<FNCellLevelInstanceLocator>& LevelInstances)
{
	CachedInitialCellLevelInstances = LevelInstances; // Copy
	OnQueryComplete.Remove(OnInitialQueryHandle);
}

void ANProcGenRelay::Server_QueryNearbyCells_Implementation(const FVector Location, const float Range, const uint32 OperationTicket)
{
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	const float ClampedRange = FMath::Min(Range, Settings->NetworkQueryMaxRange);
	
	// Find our instances
	const TArray<ANCellLevelInstance*> Instances = FNProcGenRegistry::GetCellLevelInstancesInRange(Location, ClampedRange, OperationTicket);

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
	OnQueryComplete.Broadcast(Results);
}
