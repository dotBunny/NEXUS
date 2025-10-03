// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionDetails.h"
#include "Components/BillboardComponent.h"
#include "Macros/NActorMacros.h"
#include "NCellJunctionComponent.generated.h"

class ALevelInstance;
class UNCell;

UCLASS(ClassGroup=(Nexus), meta=(BlueprintSpawnableComponent),
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

		N_WORLD_ICON_IMPLEMENTATION("/NexusProcGen/EditorResources/S_NCellJunctionComponent", this, false, 0.35f)
	}

public:

	FRotator GetOffsetRotator() const;
	FVector GetOffsetLocation() const;

	// TODO: Assess if we need to actually register them
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

#if WITH_EDITOR
	FString GetJunctionName() const;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

	virtual void PostEditImport() override;
#endif // WITH_EDITOR
	
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;
	
	UPROPERTY(EditAnywhere, Category = "NCell Junction")
	FNCellJunctionDetails Details;

private:
	ALevelInstance* LevelInstance;
	N_WORLD_ICON_HEADER()
};