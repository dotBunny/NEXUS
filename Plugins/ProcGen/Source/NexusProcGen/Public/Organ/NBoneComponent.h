// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellJunctionDetails.h"
#include "Macros/NActorMacros.h"
#include "Types/NCardinalRotation.h"
#include "NBoneComponent.generated.h"

// Bones will be anchors for junctions
// need to add something like a 'organExclusion' ? for the actual space


class UNOrganComponent;

UENUM(BlueprintType)
enum class ENBoneMode : uint8
{
	Manual = 0,
	Automatic = 1,
	Disabled = 2
};


UCLASS(ClassGroup="NEXUS", DisplayName = "Bone Component", meta=(BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNBoneComponent : public USceneComponent
{
	GENERATED_BODY()

	explicit UNBoneComponent(const FObjectInitializer& ObjectInitializer);
public:
	
#if WITH_EDITOR
	virtual void OnRegister() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
	void OnModeChanged(ENBoneMode NewMode);
	void SetAutomaticTransform();
	
#endif // WITH_EDITOR
	
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;
	
	UPROPERTY(EditInstanceOnly)
	FIntVector2 UnitSize = FIntVector2(4, 2);

	UPROPERTY(VisibleAnywhere)
	FNCardinalRotation WorldCardinalRotation;
	
	UPROPERTY(EditInstanceOnly)
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::Required;
	
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UNOrganComponent> OrganComponent;
	
	UPROPERTY(EditAnywhere)
	ENBoneMode Mode = ENBoneMode::Manual;
	
private:
	N_WORLD_ICON_HEADER()
};
	