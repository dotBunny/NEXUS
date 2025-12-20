// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Macros/NActorMacros.h"
#include "NCellProxy.generated.h"

class ANCellLevelInstance;
class ALevelInstance;
class ANCellActor;
class UNCell;
class UDynamicMeshComponent;

/**
 * A light-weight representation of the NCell+Level spawned into the world during generation.
 * It can be easily removed, but also converted into the actual prefab.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, Transient, DisplayName = "NEXUS: Cell Proxy")
class NEXUSPROCGEN_API ANCellProxy : public AActor
{
	friend class UNCellActorFactory;
	
	GENERATED_BODY()

	explicit ANCellProxy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void CreateLevelInstance();
	void LoadLevelInstance();
	void UnloadLevelInstance();
	void DestroyLevelInstance();
	
protected:
	void InitializeFromNCell(UNCell* InNCell);
private:

	void Show() const;
	void Hide() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDynamicMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNCell> NCell;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ANCellActor> NCellActor;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ANCellLevelInstance> LevelInstance;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	N_WORLD_ICON_HEADER()
};