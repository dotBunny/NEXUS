// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellAssemblyData.h"
#include "NCellJunctionDetails.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
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
class NEXUSWORLDASSEMBLY_API ANCellProxy : public AActor
{
	friend class UNCellActorFactory;
	
	GENERATED_BODY()

	explicit ANCellProxy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/**
	 * Factory for creating a cell proxy for a graph node.
	 * @param World World to spawn into.
	 * @param CellNode The graph node that sourced this cell (supplies transform and junction data).
	 * @param InstanceData Information about the generation of this cell.
	 * @param bPreLoadLevel When true, the underlying level asset begins loading immediately.
	 * @return The new proxy actor.
	 */
	static ANCellProxy* CreateInstance(UWorld* World, FNAssemblyGraphCellNode* CellNode, const FNCellAssemblyData& InstanceData, bool bPreLoadLevel = false);

	/** Spawn the paired ANCellLevelInstance so the cell's content can be loaded into the world. */
	void CreateLevelInstance();
	/** Begin loading the paired level instance's level asset. */
	void LoadLevelInstance(bool bBlocking = false);
	/** Unload the paired level instance's level asset without destroying the actor. */
	void UnloadLevelInstance(bool bTagActorsToIgnore = false) const;
	/**
	 * Destroy the paired level instance, async based on level streaming subsystem.
	 * @param bUnregisterCellLevelInstance When true, also remove the level instance from FNWorldAssemblyRegistry.
	 * @param bTagActorsToIgnore When true, tag the level instance's actors so they are skipped during subsequent operations.
	 */
	void DestroyLevelInstance(bool bUnregisterCellLevelInstance = false, bool bTagActorsToIgnore = false);
	
protected:
	/** Applies the freshly-streamed proxy material to DynamicMaterial once its async load completes. */
	void OnProxyMaterialLoaded();
	/** Configure the proxy from a UNCell asset (mesh preview, junction details, etc.). */
	void InitializeFromNCell(UNCell* InCell);
	
private:

	/** Reveal the proxy's preview mesh and enable its visualization components. */
	void Show() const;
	
	/** Hide the proxy's preview mesh — used once the paired level instance is fully loaded. */
	void Hide() const;
	
	/** Mark all actors owned by the paired level instance so they are skipped during subsequent operations. */
	void TagActorsToIgnore() const;

	/** Dynamic mesh used as the visual preview while the underlying level instance streams in. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UDynamicMeshComponent> Mesh;

	/** The cell data asset this proxy represents. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UNCell> Cell;

	/** Paired runtime level-instance actor carrying the cell's streamed content. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<ANCellLevelInstance> LevelInstance;

	/** Dynamic material applied to the proxy mesh while it stands in for the level. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
	
	
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	FNCellAssemblyData AssemblyData;
	
	/** 
	 * Per-junction data mirrored from the cell.
	 * Kept on the proxy for fast access before the level instance is available, but gets copied over when made. */
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TMap<int32, FNCellJunctionDetails> JunctionsData;

	N_WORLD_ICON_HEADER()
};