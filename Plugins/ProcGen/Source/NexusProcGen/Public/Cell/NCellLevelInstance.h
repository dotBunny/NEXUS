// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "NCellLevelInstance.generated.h"

/**
 * Runtime level-instance actor spawned for a cell during a ProcGen pass.
 *
 * Carries the replicated operation ticket so clients can attribute the cell back to its owning
 * UNProcGenOperation, and stores a pointer to the cell's junction data for quick lookup.
 * Not author-placeable — instances are created exclusively by the ProcGen pipeline.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell LevelInstance")
class NEXUSPROCGEN_API ANCellLevelInstance final : public ALevelInstance
{
	friend class ANCellProxy;

	GENERATED_BODY()

	explicit ANCellLevelInstance();

public:
	/** Local copy of junction data from generation output, built from replicated JunctionDetails array. */
	TMap<int32, FNCellJunctionDetails> JunctionData;

	//~ALevelInstance
	virtual void OnLevelInstanceLoaded() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//End ALevelInstance

	/** @return The ProcGen operation ticket this level instance belongs to. */
	uint32 GetOperationTicket() const { return OperationTicket; }
	/** @return Spawn GUID used to uniquely identify this level instance within its operation. */
	FGuid& GetLevelInstanceSpawnGuid() { return LevelInstanceSpawnGuid; }

protected:
	/** The replicated identifier that created this ANCellLevelInstance on the server. */
	UPROPERTY(Replicated)
	uint32 OperationTicket = 0;
	
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_JunctionDetails);
	TArray<FNCellJunctionDetails> JunctionDetails;
	
	UFUNCTION()
	void OnRep_JunctionDetails();
	
	/** Take the flat JunctionDetails array and produce the JunctionData instance-keyed map. **/
	void FillJunctionData();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
};
