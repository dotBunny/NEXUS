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
	return KnownOperations.IsEmpty() && HasNearbyCellLevelInstances();
}

FIntVector2 ANWorldAssemblyRelay::GetRemainingStatus() const
{
	const TArray<FNCellLevelInstanceLocator> RemainingLocators = FNWorldAssemblyRegistry::GetRemainingCellLevelInstancesToSync(CachedNearbyCellLevelInstances);
	const int RemainingCount = RemainingLocators.Num();
	return FIntVector2(RemainingCount, CachedNearbyCellLevelInstances.Num());
}

bool ANWorldAssemblyRelay::HasNearbyCellLevelInstances()
{
	// Early out the best we can
	if (bHasNearbyCellLevelInstances) return true;
	//if (CachedNearbyCellLevelInstances.IsEmpty())  return false;

	// If we haven't got them lets reupdate and respond accordingly
	bHasNearbyCellLevelInstances = FNWorldAssemblyRegistry::HasCellLevelInstances(CachedNearbyCellLevelInstances);
	return bHasNearbyCellLevelInstances;
}

void ANWorldAssemblyRelay::BeginPlay()
{
	Super::BeginPlay();

	// Link to world Subsystem regardless of ownership
	// We will use this at times to callback to things on a client that get subscribed.
	Subsystem = UNWorldAssemblySubsystem::Get(GetWorld());

	// Owner may already be set here (listen-server / owner replicated before BeginPlay).
	// When it replicates afterwards on a client, OnRep_Owner drives the same path.
	TryRegisterAsLocalRelay();
}

void ANWorldAssemblyRelay::OnRep_Owner()
{
	Super::OnRep_Owner();

	// On owner-only actors the Owner property can replicate after BeginPlay (or during the initial
	// bunch before it), so registration is anchored to whenever the owner actually resolves.
	TryRegisterAsLocalRelay();
}

void ANWorldAssemblyRelay::TryRegisterAsLocalRelay()
{
	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC == nullptr || !OwningPC->IsLocalController()) return;

	// OnRep_Owner can fire during the initial bunch before BeginPlay, so resolve the subsystem lazily.
	if (Subsystem == nullptr)
	{
		Subsystem = UNWorldAssemblySubsystem::Get(GetWorld());
	}
	if (Subsystem == nullptr) return;

	// Idempotent: re-registering the same relay is a no-op, so both entry points can call this safely.
	Subsystem->RegisterLocalRelay(this);

	// Initial check
	UpdateNearbyCells();
}

void ANWorldAssemblyRelay::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC != nullptr && OwningPC->IsLocalController())
	{
		if (Subsystem != nullptr)
		{
			Subsystem->UnregisterLocalRelay(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANWorldAssemblyRelay::UpdateNearbyCells(bool bIsLevelLoaded)
{
	// Coalesce: while a request is awaiting its server reply, fold this call into a single trailing
	// refresh instead of dispatching another reliable RPC. Operation bursts (e.g. several finishing
	// at once via Client_OperationFinished) would otherwise fire one full-registry query each.
	if (bNearbyCellsRequestInFlight)
	{
		bNearbyCellsUpdatePending = true;
		bPendingIsLevelLoaded = bIsLevelLoaded;
		return;
	}

	bHasNearbyCellLevelInstances = false;
	CachedNearbyCellLevelInstances.Reset();

	// Setup initial query
	const APlayerController* OwningPC = Cast<APlayerController>(GetOwner());
	if (OwningPC == nullptr) return;

	FVector Location  = OwningPC->GetSpawnLocation();
	APawn* SafePawn = OwningPC->GetPawnOrSpectator();
	if (SafePawn!= nullptr)
	{
		Location = SafePawn->GetActorLocation();
	}
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Request nearby(%s) ANCellLevelInstances list."), *Location.ToString());

	// Mark in-flight only at dispatch so an early-out above can never wedge the guard.
	bNearbyCellsRequestInFlight = true;
	Server_RequestNearbyCells(Location, 0, bIsLevelLoaded);
}

void ANWorldAssemblyRelay::Client_OperationFinished_Implementation(const int32 OperationTicket)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received finished notification for operation(%i)."), OperationTicket);
	KnownOperations.RemoveSwap(OperationTicket);

	UpdateNearbyCells(true);
}

void ANWorldAssemblyRelay::Client_OperationStarted_Implementation(const int32 OperationTicket)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received starting notification for operation(%i)."), OperationTicket);
	KnownOperations.AddUnique(OperationTicket);
}

void ANWorldAssemblyRelay::Client_OperationDestroyed_Implementation(const int32 OperationTicket)
{
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Received destruction notification for operation(%i)."), OperationTicket);
	KnownOperations.RemoveSwap(OperationTicket);
}


void ANWorldAssemblyRelay::Server_RequestNearbyCells_Implementation(const FVector Location, const int32 OperationTicket, const bool bIsLevelLoaded)
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
	bHasNearbyCellLevelInstances = false;
	CachedNearbyCellLevelInstances = Results; // Copy

	// Request answered; fire the single trailing refresh if one coalesced while we were waiting.
	bNearbyCellsRequestInFlight = false;
	if (bNearbyCellsUpdatePending)
	{
		bNearbyCellsUpdatePending = false;
		UpdateNearbyCells(bPendingIsLevelLoaded);
	}
}
