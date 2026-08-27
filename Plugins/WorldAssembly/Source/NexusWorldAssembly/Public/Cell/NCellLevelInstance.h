// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NCellAssemblyData.h"
#include "NCellJunctionDetails.h"
#include "Collections/NGameplayTagCounter.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "NCellLevelInstance.generated.h"

class UDynamicMesh;

/**
 * Runtime level-instance actor spawned for a cell during a World Assembly pass.
 *
 * Carries the replicated operation ticket so clients can attribute the cell back to its owning
 * UNAssemblyOperation, and stores a pointer to the cell's junction data for quick lookup.
 * Not author-placeable — instances are created exclusively by the World Assembly pipeline.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-level-instance/">ANCellLevelInstance</a>
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

	/** @return Mutable access to the context tags recorded on this cell's assembly data. */
	FGameplayTagContainer& GetContextTags()
	{
		return AssemblyData.ContextTags;
	}

	FGameplayTagContainer& GetContextTagsAdded()
	{
		return AssemblyData.ContextTagsAdded;
	}

	TArray<FNGameplayTagCount>& GetTagCounterArray()
	{
		return AssemblyData.TagCounter;
	}

	TMap<FGameplayTag, int32> GetTagCounter() const
	{
		return FNGameplayTagCounter(AssemblyData.TagCounter).GameplayTags;
	}

	bool IsHotPath() const
	{
		return AssemblyData.bHotPathShortest || AssemblyData.bHotPathSequential;
	}

	bool IsHotPathShortest() const
	{
		return AssemblyData.bHotPathShortest;
	}

	bool IsHotPathSequential() const
	{
		return AssemblyData.bHotPathSequential;
	}

	/**
	 * @return How many cells separate this one from the hot path, taking whichever variant runs nearer — the
	 *         numeric counterpart of IsHotPath, and 0 exactly when it returns true.
	 */
	uint8 GetHotPathScore() const
	{
		return FMath::Min(AssemblyData.HotPathShortestScore, AssemblyData.HotPathSequentialScore);
	}

	/** @return Hops in cells to the nearest cell on the shortest-path hot path; 0 when this cell is on it. */
	uint8 GetHotPathShortestScore() const
	{
		return AssemblyData.HotPathShortestScore;
	}

	/** @return Hops in cells to the nearest cell on the sequential hot path; 0 when this cell is on it. */
	uint8 GetHotPathSequentialScore() const
	{
		return AssemblyData.HotPathSequentialScore;
	}

	/** @return Hops in cells to the nearest Important-flagged cell; 0 when this cell carries the tag itself. */
	uint8 GetImportanceScore() const
	{
		return AssemblyData.ImportanceScore;
	}

	/**
	 * @param JunctionIdentifier The junction to test.
	 * @return true when the cell across that junction sits nearer the hot path than this one does — the doorway to
	 *         take when heading for the route.
	 * @note Answered from the score carried on the link rather than by resolving the far cell, which frequently is
	 *       not streamed in. False for an unconnected junction, for one reaching a bone, and for a junction whose
	 *       neighbour is equally near or further, so a cell already on the route reports false in every direction.
	 */
	bool DoesJunctionLeadTowardHotPath(int32 JunctionIdentifier);

	/**
	 * @param JunctionIdentifier The junction to test.
	 * @return true when the cell across that junction sits nearer an Important-flagged cell than this one does.
	 */
	bool DoesJunctionLeadTowardImportant(int32 JunctionIdentifier);

	/** @return Mutable access to the assembly tags recorded on this cell's assembly data. */
	FGameplayTagContainer& GetAssemblyTags()
	{
		return AssemblyData.AssemblyTags;
	}

	/** @return The seed recorded on this cell's assembly data. */
	uint64 GetSeed() const
	{
		return AssemblyData.Seed;
	}

	/** @return The graph node identifier recorded on this cell's assembly data. */
	int32 GetNodeIdentifier() const
	{
		return AssemblyData.NodeIdentifier;
	}

	/** @return The cell's proxy dynamic mesh, or nullptr if none is set; only resolvable on the server/owner that spawned the proxy. */
	UDynamicMesh* GetProxyMesh() const;

	/** Caches the proxy's dynamic mesh on this cell; called on the server/owner during ANCellProxy spawn. @param Mesh The proxy dynamic mesh. */
	void SetProxyMesh(UDynamicMesh* Mesh);

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
	int32 GetOperationTicket() const { return AssemblyData.OperationTicket; }

	/** @return Spawn GUID used to uniquely identify this level instance within its operation. */
	FGuid& GetLevelInstanceSpawnGuid() { return LevelInstanceSpawnGuid; }

	void UpdateFromAssemblyData();

	FNCellLinkDetails GetCellLinkDetails(int32 JunctionIdentifier);

protected:

	/** Replicated post-assembly data for this cell instance; applied on clients via OnRep_AssemblyData. */
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_AssemblyData)
	FNCellAssemblyData AssemblyData;

	/** RepNotify for AssemblyData: refreshes this instance from the newly received data. */
	UFUNCTION()
	void OnRep_AssemblyData();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Backstop unregister. EndPlay is the normal path, but the engine does not route it on every teardown
	 * (editor undo, forced delete, or GC of an actor that never begun play), and this actor is not GC-rooted,
	 * so without this it could be freed while still in the registry — leaving a dangling pointer that
	 * FNWorldAssemblyRegistry::OnPostWorldCleanup would dereference. BeginDestroy always runs before the
	 * actor's memory is reclaimed, so it is the reliable place to guarantee removal.
	 */
	virtual void BeginDestroy() override;

	/** Whether this cell is currently registered with the World Assembly registry. */
	bool bRegistered = false;

	/** Non-owning reference to the originating ANCellProxy's dynamic mesh; set on the server/owner at spawn for GetProxyMesh. */
	TWeakObjectPtr<UDynamicMesh> ProxyMesh;
};
