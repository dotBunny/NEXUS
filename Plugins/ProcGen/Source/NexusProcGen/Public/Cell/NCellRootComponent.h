// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellRootDetails.h"
#include "NCellRootComponent.generated.h"

class ALevelInstance;
class ANCellActor;
class UNCell;

UCLASS(ClassGroup="NEXUS", DisplayName = "Cell Root Component", HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNCellRootComponent : public USceneComponent
{
	friend class ANCellActor;
	
	GENERATED_BODY()
	
	explicit UNCellRootComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
	{
		LevelInstance = nullptr;
		PrimaryComponentTick.bCanEverTick = false;
		PrimaryComponentTick.bStartWithTickEnabled = false;
		bAutoActivate = 0;
	}

public:
	FORCEINLINE ANCellActor* GetNCellActor() const;
	
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI, const uint8 DrawVoxelMode) const;
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI, const FBox& WorldBoundsBox,  const FLinearColor& BoundsColor, const TArray<FVector>& WorldHullVertices, const FLinearColor& HullColor, const uint8 DrawVoxelMode) const;

	FRotator GetOffsetRotator() const;
	FVector GetOffsetLocation() const;
	
	FBox GetOffsetBounds() const;

	// TODO: Assess if we need to actually register them
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	
	bool IsRootComponent() const
	{
		return GetOwner() != nullptr && GetOwner()->GetRootComponent() == this;
	}


	

	// TODO: DrawDebugHelper (Draw at runtime)


	void Reset();
	/**
	 * Details about the NBlock that is used during the generation process.
	 * This information is synced with a separate file as to ensure that we only load asset data when needed.
	 */
	UPROPERTY(EditAnywhere, Category = "NCell Root")
	FNCellRootDetails Details;

private:
	ALevelInstance* LevelInstance;
};
