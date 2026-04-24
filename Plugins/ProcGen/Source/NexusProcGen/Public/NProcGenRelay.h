// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellLevelInstanceLocator.h"
#include "GameFramework/Actor.h"
#include "NProcGenRelay.generated.h"

/**
 * Per-player networking helper spawned by the ProcGen subsystem.
 *
 * Lives invisibly on every client as the server's handle for that player — the server pushes
 * operation lifecycle events into it (Started/Finished/Destroyed) and answers "what level
 * instances are around this player right now?" via the nearby-cells RPC exchange.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Relay")
class NEXUSPROCGEN_API ANProcGenRelay : public AActor
{
	GENERATED_BODY()

	explicit ANProcGenRelay();

public:

	/** @return true once the relay has enough state to participate in cell-level-instance correlation. */
	bool IsReady();

	/**
	 * Refresh the cached nearby-cell list by re-asking the server.
	 * @param bIsLevelLoaded Whether only loaded cell levels should be considered.
	 */
	void UpdateNearbyCells(bool bIsLevelLoaded = true);

	/** Notify the client that an operation has started on the server. */
	UFUNCTION(Client, Reliable)
	void Client_OperationStarted(uint32 OperationTicket);

	/** Notify the client that an operation has completed. */
	UFUNCTION(Client, Reliable)
	void Client_OperationFinished(uint32 OperationTicket);

	/** Notify the client that an operation has been destroyed. */
	UFUNCTION(Client, Reliable)
	void Client_OperationDestroyed(uint32 OperationTicket);

protected:
	//~AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//End AActor

	/**
	 * Ask the server for ANCellLevelInstances within Range of Location.
	 * @param Location World-space point to search from.
	 * @param OperationTicket Filter to one operation, or 0 for all.
	 * @param bIsLevelLoaded Is the level loaded attached to the ANCellLevelInstance?
	 * @remark Must be called on the owning client. On the server it is a no-op on remote relays.
	 */
	UFUNCTION(Server, Reliable)
	void Server_RequestNearbyCells(FVector Location, uint32 OperationTicket, bool bIsLevelLoaded = true);

	/** Server response payload: cell-instance locators within range of the client's request. */
	UFUNCTION(Client, Reliable)
	void Client_ReceiveNearbyCells(const TArray<FNCellLevelInstanceLocator>& Results);

private:
	/** Handle for the delegate wiring this relay into the registry's nearby-request pipeline. */
	FDelegateHandle OnRequestNearbyHandle;

	/** True when CachedNearbyCellLevelInstances has been populated by the server at least once. */
	bool bHasNearbyCellLevelInstances = false;

	/** Most recently received nearby-cell payload. */
	TArray<FNCellLevelInstanceLocator> CachedNearbyCellLevelInstances;

	/** Operation tickets the client has been informed about. */
	TArray<uint32> KnownOperations;

	/** @return true if a nearby-cell payload is currently cached. */
	bool HasNearbyCellLevelInstances();
};
