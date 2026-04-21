// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellRootComponent.h"
#include "NCellActor.generated.h"

class UNCellJunctionComponent;
class ANCellLevelInstance;

/** Broadcast after InitializeFromProxy has finished applying data from a cell proxy. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializedFromProxy);

/**
 * The persistent in-level representation of a cell.
 *
 * A cell actor is locked to its level's origin, hosts the UNCellRootComponent as its root, and owns the
 * map of junction components that define its connectivity.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell Actor", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input, LevelInstance, WorldPartition, DataLayers, Rendering, LOD, HLOD, Physics,
	Collision, Networking, Replication))
class NEXUSPROCGEN_API ANCellActor : public AActor
{
	friend class UAssetValidator_World;
	friend class UAssetDefinition_NCell;
	friend class FNProcGenEditorUtils;
	friend class FNProcGenEditorCommands;
	friend class FNProcGenEditorUndo;
	friend class FDebugRenderSceneProxy;
	friend class UNCellDebugDrawComponent;
	friend class UNCellJunctionComponent;

	GENERATED_BODY()
	
	explicit ANCellActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
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
#endif // WITH_EDITOR
		
		RootComponent->Mobility = EComponentMobility::Static;
	}
public:
	/** @return true if the cell has unsaved author-time changes (bounds/hull/voxel/junctions). */
	bool IsActorDirty() const { return bActorDirty; }
	/** Mark or clear the dirty flag; marking dirty also dirties the owning package so it is saved. */
	void SetActorDirty(const bool bIsDirty = true)
	{
		if (bIsDirty)
		{
			if (!bActorDirty)
			{
				// We need to mark the outer package (level) dirty for the actor

				// ReSharper disable once CppExpressionWithoutSideEffects
				MarkPackageDirty();
				bActorDirty = true;
			}
		}
		else
		{
			if (bActorDirty)
			{
				// We can't actually unmark a package (without saving?), so we just get rid of our flag.
				bActorDirty = false;
			}
		}
	}
	/** @return true if this cell actor was spawned by the proxy pipeline rather than placed at author time. */
	bool WasSpawnedFromProxy() const { return bSpawnedFromProxy; }

	/**
	 * Populate this cell actor's transient state from the supplied level instance proxy.
	 * @param LevelInstance The ANCellLevelInstance that owns this actor at runtime.
	 */
	void InitializeFromProxy(ANCellLevelInstance* LevelInstance);

	/** Broadcast after InitializeFromProxy completes, so Blueprints can react to post-spawn initialization. */
	UPROPERTY(BlueprintAssignable)
	FOnInitializedFromProxy OnInitializedFromProxy;
	
#if WITH_EDITOR
	/** Purposely monitor for any sort of movement and reset it back to origin */
	
	virtual bool IsUserManaged() const override { return false; }
	virtual void PostEditMove(bool bFinished) override;
	virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
	virtual bool ShouldExport() override { return false; } // Stops Copy/Paste/Cut/Duplicate
	int32 GetCellJunctionNextIdentifier() { return CellJunctionNextIdentifier++; }
	virtual void PostRegisterAllComponents() override;
	bool HasDifferencesFromSidecar() const;
	
	
	
#endif // WITH_EDITOR

	/** Recompute the cell's bounds from its level content and store the result on the root component. */
	void CalculateBounds();
	/**
	 * Recompute the cell's convex hull from its level content.
	 * @param bUpdateDerivedData When true, dependent derived data (voxels) is also refreshed.
	 */
	void CalculateHull(bool bUpdateDerivedData = true);
	/** Recompute the cell's voxel data using the current bounds/hull and project voxel settings. */
	void CalculateVoxelData();

	/** @return The cell's root component. */
	UNCellRootComponent* GetCellRoot() const { return CellRoot; }

protected:
	/** The root component for this cell; also the actor's root component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNCellRootComponent> CellRoot;

	/** Junction components owned by this cell, keyed by the junction identifier. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<int32, TObjectPtr<UNCellJunctionComponent>> CellJunctions;

private:
	/** This is something that needs to be turned off when we spawn */
	UPROPERTY(EditInstanceOnly)
	TArray<TObjectPtr<AActor>> AuthorTimeActors;

	/** Reference to the paired UNCell data asset that stores derived cell data on disk. */
	UPROPERTY(VisibleInstanceOnly)
	TSoftObjectPtr<UNCell> Sidecar = nullptr;

	/** Next identifier to hand out when a new junction is added to this cell. */
	UPROPERTY(VisibleInstanceOnly)
	int32 CellJunctionNextIdentifier = 0;

	/** true when the actor has author-time changes that have not yet been saved. */
	bool bActorDirty = false;
	/** true when this actor was spawned by the proxy pipeline rather than placed manually. */
	bool bSpawnedFromProxy = false;
};