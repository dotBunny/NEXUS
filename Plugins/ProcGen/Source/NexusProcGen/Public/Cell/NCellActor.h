// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellRootComponent.h"
#include "Components/BillboardComponent.h"
#include "Macros/NActorMacros.h"
#include "NCellActor.generated.h"

class UNCellJunctionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializedFromProxy);

UCLASS(NotPlaceable, HideDropdown, Hidden, DisplayName = "NCell Actor", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input, LevelInstance, WorldPartition, DataLayers, Rendering, LOD, HLOD, Physics,
	Collision, Networking, Replication))
class NEXUSPROCGEN_API ANCellActor : public AActor
{
	friend class UAssetValidator_World;
	friend class UAssetDefinition_NCell;
	friend class FNProcGenEditorUtils;
	friend class FNProcGenEditorCommands;
	friend class FDebugRenderSceneProxy;
	friend class UNCellDebugDrawComponent;
	friend class UNCellJunctionComponent;
	
	GENERATED_BODY()
	
	explicit ANCellActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
	{
		PrimaryActorTick.bCanEverTick = false;
		PrimaryActorTick.bStartWithTickEnabled = false;
		
		// Add our root component
		CellRoot = CreateDefaultSubobject<UNCellRootComponent>(TEXT("NCellRoot"));
		SetRootComponent(CellRoot);

		// Lock it down
#if WITH_EDITOR		
		SetLockLocation(true);
		bCanPlayFromHere = 0;
#endif		
		
		RootComponent->Mobility = EComponentMobility::Static;
		
		N_WORLD_ICON_IMPLEMENTATION("/NexusProcGen/EditorResources/S_NCellActor", RootComponent, true)
	}
public:
	bool IsActorDirty() const { return bActorDirty; }
	void SetActorDirty()
	{
		bActorDirty = true;
	}
	bool WasSpawnedFromProxy() const { return bSpawnedFromProxy; }

	void InitializeFromProxy();

	UPROPERTY(BlueprintAssignable)
	FOnInitializedFromProxy OnInitializedFromProxy;
	
#if WITH_EDITOR
	/**
	 *  Purposely monitor for any sort of movement and reset it back to origin
	 */
	virtual void PostEditMove(bool bFinished) override;
	virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
	virtual bool ShouldExport() override { return false; } // Stops Copy/Paste/Cut/Duplicate
	int32 GetCellJunctionNextIdentifier() { return CellJunctionNextIdentifier++; }
#endif	

	void CalculateBounds();
	void CalculateHull();
	
	UNCellRootComponent* GetCellRoot() const { return CellRoot; }
	
protected:
	/**
	 * This is something that needs to be turned off when we spawn
	 */
	UPROPERTY(EditInstanceOnly)
	TArray<TObjectPtr<AActor>> EditorOnlyActors;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNCellRootComponent> CellRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<int32, TObjectPtr<UNCellJunctionComponent>> CellJunctions;
	
	UPROPERTY(VisibleInstanceOnly)
	TSoftObjectPtr<UNCell> Sidecar = nullptr;

	UPROPERTY(VisibleInstanceOnly)
	int32 CellJunctionNextIdentifier = 0;
	
private:
	bool bActorDirty = false;
	bool bSpawnedFromProxy = false;

	N_WORLD_ICON_HEADER()
};