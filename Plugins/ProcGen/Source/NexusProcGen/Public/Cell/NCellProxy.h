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
	
	static ANCellProxy* CreateInstance(UWorld* World, const uint32& OperationTicket, const FNProcGenGraphCellNode* CellNode, bool bPreLoadLevel = true);
	
	void CreateLevelInstance();
	void LoadLevelInstance();
	void UnloadLevelInstance() const;
	void DestroyLevelInstance();
	
protected:
	void OnProxyMaterialLoaded();
	void InitializeFromNCell(UNCell* InNCell);
	void InitializeFromCellNode(const FNProcGenGraphCellNode* CellNode);

private:

	void Show() const;
	void Hide() const;
	void CreateCollisionMesh() const;

	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UDynamicMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UNCell> NCell;

	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<ANCellLevelInstance> LevelInstance;
	
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	uint32 OperationTicket = 0;

	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// TODO: if we want to replicate we need to make a FastArraySerializer
	UPROPERTY(VisibleAnywhere, Category = "Cell Proxy")
	TMap<int32, FNCellJunctionDetails> JunctionsData;
	
	N_WORLD_ICON_HEADER()
};