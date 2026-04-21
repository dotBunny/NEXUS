// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Macros/NActorMacros.h"
#include "NCellProxy.generated.h"

class ANCellLevelInstance;
class ALevelInstance;
class ANCellActor;
class UNCell;
class UDynamicMeshComponent;

/**
 * A light-weight representation of the NCell+Level spawned into the world during generation.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell Proxy")
class NEXUSPROCGEN_API ANCellProxy : public AActor
{
	friend class UNCellActorFactory;
	
	GENERATED_BODY()

	explicit ANCellProxy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/**
	 * Factory for creating a cell proxy for a graph node.
	 * @param World World to spawn into.
	 * @param OperationTicket Owning operation's ticket, stored on the proxy for later lookup.
	 * @param CellNode The graph node that sourced this cell (supplies transform and junction data).
	 * @param bPreLoadLevel When true, the underlying level asset begins loading immediately.
	 * @return The new proxy actor.
	 */
	static ANCellProxy* CreateInstance(UWorld* World, const uint32& OperationTicket, const FNProcGenGraphCellNode* CellNode, bool bPreLoadLevel = true);

	/** Spawn the paired ANCellLevelInstance so the cell's content can be loaded into the world. */
	void CreateLevelInstance();
	/** Begin loading the paired level instance's level asset. */
	void LoadLevelInstance();
	/** Unload the paired level instance's level asset without destroying the actor. */
	void UnloadLevelInstance() const;
	/**
	 * Destroy the paired level instance.
	 * @param bUnregisterCellLevelInstance When true, also remove the level instance from FNProcGenRegistry.
	 */
	void DestroyLevelInstance(bool bUnregisterCellLevelInstance = false);

protected:
	void OnProxyMaterialLoaded();
	void InitializeFromNCell(UNCell* InNCell);
	void InitializeFromCellNode(const FNProcGenGraphCellNode* CellNode);

private:

	void Show() const;
	void Hide() const;
	void CreateCollisionMesh() const;

	/** Dynamic mesh used as the visual preview while the underlying level instance streams in. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UDynamicMeshComponent> Mesh;

	/** The cell data asset this proxy represents. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UNCell> NCell;

	/** Paired runtime level-instance actor carrying the cell's streamed content. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<ANCellLevelInstance> LevelInstance;

	/** Ticket of the owning UNProcGenOperation; used to group proxies per pass in the registry. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	uint32 OperationTicket = 0;

	/** Dynamic material applied to the proxy mesh while it stands in for the level. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// TODO: if we want to replicate we need to make a FastArraySerializer
	/** 
	 * Per-junction data mirrored from the cell.
	 * Kept on the proxy for fast access before the level instance is available, but gets copied over when made. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TMap<int32, FNCellJunctionDetails> JunctionsData;

	N_WORLD_ICON_HEADER()
};