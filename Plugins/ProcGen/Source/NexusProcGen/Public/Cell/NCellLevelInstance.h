// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "NCellLevelInstance.generated.h"

UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell LevelInstance")
class NEXUSPROCGEN_API ANCellLevelInstance final : public ALevelInstance
{
	friend class ANCellProxy;
	
	GENERATED_BODY()

	explicit ANCellLevelInstance();
	
public:
	TMap<int32, FNCellJunctionDetails>* JunctionData;
	
	virtual void OnLevelInstanceLoaded() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	uint32 GetOperationTicket() const { return OperationTicket; }

protected:
	/**
	 * The replicated identifier that created this ANCellLevelInstance on the server.
	 */
	UPROPERTY(Replicated)
	uint32 OperationTicket = 0;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
