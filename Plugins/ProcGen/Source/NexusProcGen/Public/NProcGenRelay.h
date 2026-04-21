// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellLevelInstanceLocator.h"
#include "GameFramework/Actor.h"
#include "NProcGenRelay.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNCellLevelInstanceQueryComplete, const TArray<FNCellLevelInstanceLocator>&);

UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Relay")
class NEXUSPROCGEN_API ANProcGenRelay : public AActor
{
	GENERATED_BODY()

	explicit ANProcGenRelay();

public:
	/**
	 * Fires when a query response arrives.	 
	 */
	FOnNCellLevelInstanceQueryComplete OnQueryComplete;
	
	bool HasInitialCellLevelInstances();

protected:
	//~AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//End AActor

	/**
	 * Ask the server for ANCellLevelInstances within Range of Location.
	 * @param Location World-space point to search from.
	 * @param Range Maximum distance (cm). The server clamps this against UNProcGenSettings::NetworkQueryMaxRange.
	 * @param OperationTicket Filter to one operation, or 0 for all.	 
	 * @remark Must be called on the owning client. On the server it is a no-op on remote relays.
	 */
	UFUNCTION(Server, Reliable)
	void Server_QueryNearbyCells(FVector Location, float Range, uint32 OperationTicket);

	UFUNCTION(Client, Reliable)
	void Client_ReceiveNearbyCells(const TArray<FNCellLevelInstanceLocator>& Results);

	UFUNCTION()
	void OnInitialQueryComplete(const TArray<FNCellLevelInstanceLocator>& LevelInstances);
	
private:
	FDelegateHandle OnInitialQueryHandle;
	bool bHasInitialCellLevelInstances = false;
	TArray<FNCellLevelInstanceLocator> CachedInitialCellLevelInstances;
	
};
