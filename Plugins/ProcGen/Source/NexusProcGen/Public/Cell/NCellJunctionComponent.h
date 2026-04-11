// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "Components/BillboardComponent.h"
#include "Macros/NActorMacros.h"
#include "NCellJunctionComponent.generated.h"

class UNCellRootComponent;
class ALevelInstance;
class UNCell;

UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Cell Junction", meta=(BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNCellJunctionComponent : public USceneComponent
{
	GENERATED_BODY()
	
	explicit UNCellJunctionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Details()
		, LevelInstance(nullptr)
	{
		PrimaryComponentTick.bCanEverTick = false;
		PrimaryComponentTick.bStartWithTickEnabled = false;
		bAutoActivate = 0;
		
		
		Mobility = EComponentMobility::Static;
#if WITH_EDITOR
		TArray<USceneComponent*> ParentComponents;
		this->GetParentComponents(ParentComponents);
		for (USceneComponent* Parent : ParentComponents)
		{
			if (Parent)
			{
				Parent->TransformUpdated.AddUObject(this, &UNCellJunctionComponent::OnTransformUpdated);
			}
		}
		TransformUpdated.AddUObject(this, &UNCellJunctionComponent::OnTransformUpdated);
#endif // WITH_EDITOR

		N_WORLD_ICON_IMPLEMENTATION_SCENE_COMPONENT("/NexusProcGen/EditorResources/S_NCellJunctionComponent", this, false, 0.35f)
	}

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NCell Junction")
	FNCellJunctionDetails Details;

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	
	FRotator GetOffsetRotator() const;
	FVector GetOffsetLocation() const;
	
	TArray<FVector> GetCornerPoints(const FVector2D& SocketUnitSize) const;
	FLinearColor GetColor() const;

#if WITH_EDITOR
	
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void PostEditImport() override;
	
	FString GetJunctionName() const;
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
	
	
	
#endif // WITH_EDITOR
	
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;
	void UpdateHullDerivedData(const UNCellRootComponent* RootComponent);
	
private:
	ALevelInstance* LevelInstance;
	N_WORLD_ICON_HEADER()
};