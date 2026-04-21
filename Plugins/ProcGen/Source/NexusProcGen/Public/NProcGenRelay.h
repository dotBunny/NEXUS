// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellLevelInstanceLocator.h"
#include "GameFramework/Actor.h"
#include "NProcGenRelay.generated.h"

UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Relay")
class NEXUSPROCGEN_API ANProcGenRelay : public AActor
{
	GENERATED_BODY()

	explicit ANProcGenRelay();

public:

	bool IsReady();
	
	int32 GetPlayerIdentifier() const { return CachedPlayerIdentifier; }
	
	void UpdateNearbyCells();

	UFUNCTION(Client, Reliable)
	void Client_OperationStarted(uint32 OperationTicket);
	
	UFUNCTION(Client, Reliable)
	void Client_OperationFinished(uint32 OperationTicket);
	
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

	UFUNCTION(Client, Reliable)
	void Client_ReceiveNearbyCells(const TArray<FNCellLevelInstanceLocator>& Results);

private:
	FDelegateHandle OnRequestNearbyHandle;
	bool bHasNearbyCellLevelInstances = false;
	TArray<FNCellLevelInstanceLocator> CachedNearbyCellLevelInstances;
	int32 CachedPlayerIdentifier = -1;
	TArray<uint32> KnownOperations;
	
	bool HasNearbyCellLevelInstances();
};
