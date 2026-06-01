// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NCellAssemblyData.h"
#include "NCellJunctionDetails.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "NCellLevelInstance.generated.h"

/**
 * Runtime level-instance actor spawned for a cell during a World Assembly pass.
 *
 * Carries the replicated operation ticket so clients can attribute the cell back to its owning
 * UNAssemblyOperation, and stores a pointer to the cell's junction data for quick lookup.
 * Not author-placeable — instances are created exclusively by the World Assembly pipeline.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell LevelInstance")
class NEXUSWORLDASSEMBLY_API ANCellLevelInstance final : public ALevelInstance
{
	friend class ANCellProxy;
	friend class FNWorldAssemblyRegistry;

	GENERATED_BODY()

	explicit ANCellLevelInstance();

public:
	/** Local copy of junction data from generation output, built from replicated JunctionDetails array. */
	TMap<int32, FNCellJunctionDetails> JunctionData;

	//~ALevelInstance
	virtual void OnLevelInstanceLoaded() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//End ALevelInstance
	
	FNCellAssemblyData& GetAssemblyData()
	{
		return AssemblyData;
	}
	
	FGameplayTagContainer& GetContextTags()
	{
		return AssemblyData.ContextTags;
	}
	
	FGameplayTagContainer& GetAssemblyTags()
	{
		return AssemblyData.AssemblyTags;
	}
	
	uint64 GetSeed() const
	{
		return AssemblyData.Seed;
	}
	
	int32 GetNodeIdentifier() const
	{
		return AssemblyData.NodeIdentifier;
	}
	
#if WITH_EDITOR
	virtual bool IsUserManaged() const override { return false; }
	virtual bool CanEnterEdit(FText* OutReason = nullptr) const override
	{
		if (OutReason)
		{
			*OutReason = NSLOCTEXT("NexusWorldAssembly",
				"CellLevelInstance_NotEditable",
				"Cell level instances are managed by World Assembly and cannot be edited in place (for now).");
		}
		return false;
	}
#endif // WITH_EDITOR

	/** @return The World Assembly operation ticket this level instance belongs to. */
	uint32 GetOperationTicket() const { return AssemblyData.OperationTicket; }
	
	/** @return Spawn GUID used to uniquely identify this level instance within its operation. */
	FGuid& GetLevelInstanceSpawnGuid() { return LevelInstanceSpawnGuid; }

protected:
	UPROPERTY(Replicated)
	FNCellAssemblyData AssemblyData;;
	
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_JunctionDetails);
	TArray<FNCellJunctionDetails> JunctionDetails;
	
	UFUNCTION()
	void OnRep_JunctionDetails();
	
	/** Take the flat JunctionDetails array and produce the JunctionData instance-keyed map. **/
	void FillJunctionData();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Whether this cell is currently registered with the World Assembly registry. */
	bool bRegistered = false;
};
