// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellRootDetails.h"
#include "NCellRootComponent.generated.h"

class ALevelInstance;
class ANCellActor;
class UNCell;

/**
 * Scene component that anchors a cell's level to the ProcGen system.
 *
 * Carries the cell's FNCellRootDetails (bounds, hull, voxel data, offsets) and registers itself with
 * the FNProcGenRegistry so runtime/editor systems can discover cells by level.
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Cell Root", HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNCellRootComponent : public USceneComponent
{
	friend class ANCellActor;
	
	GENERATED_BODY()
	
	explicit UNCellRootComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: LevelInstance(nullptr)
	{
		PrimaryComponentTick.bCanEverTick = false;
		PrimaryComponentTick.bStartWithTickEnabled = false;
		bAutoActivate = 0;
	}

public:
	/** @return The owning ANCellActor, or nullptr if the component is not owned by a cell actor. */
	FORCEINLINE ANCellActor* GetNCellActor() const;

	/** Draw the cell's debug bounds, hull and voxels using the current stored Details. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI, const uint8 DrawVoxelMode) const;
	/** Draw the cell's debug bounds, hull and voxels using an override world-space bounds/hull. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI, const FBox& WorldBoundsBox,  const FLinearColor& BoundsColor, const TArray<FVector>& WorldHullVertices, const FLinearColor& HullColor, const uint8 DrawVoxelMode) const;

	/** @return The offset rotator authored on this root component's details. */
	FRotator GetOffsetRotator() const;
	/** @return The offset location authored on this root component's details. */
	FVector GetOffsetLocation() const;

	/** @return Bounds transformed by the authored offset location/rotation. */
	FBox GetOffsetBounds() const;

	//~USceneComponent
	// TODO: Assess if we need to actually register them
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//End USceneComponent

	/** @return true if this component is its owning actor's root component. */
	bool IsRootComponent() const
	{
		return GetOwner() != nullptr && GetOwner()->GetRootComponent() == this;
	}

	/**
	 * Details about the NBlock that is used during the generation process.
	 * This information is synced with a separate file as to ensure that we only load asset data when needed.
	 */
	UPROPERTY(EditAnywhere, Category = "NCell Root")
	FNCellRootDetails Details;

private:
	/** Cached level-instance owner; set when the component is spawned as part of a streamed-in cell level. */
	ALevelInstance* LevelInstance;
};
