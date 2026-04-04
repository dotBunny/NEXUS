// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenSettings.h"
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


UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Bone", meta=(BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNBoneComponent : public USceneComponent
{
	GENERATED_BODY()

	explicit UNBoneComponent(const FObjectInitializer& ObjectInitializer);

public:
	
	UPROPERTY(EditInstanceOnly, Category = "Bone Component")
	FIntVector2 SocketSize = FIntVector2(4, 4);

	UPROPERTY(VisibleAnywhere, Category = "Bone Component")
	FNCardinalRotation WorldCardinalRotation;
	
	UPROPERTY(EditInstanceOnly, Category = "Bone Component")
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	UPROPERTY(EditInstanceOnly, Category = "Bone Component")
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::Required;
	
	UPROPERTY(EditAnywhere, Category = "Bone Component")
	ENBoneMode Mode = ENBoneMode::Automatic;
	
	UPROPERTY()
	TObjectPtr<UNOrganComponent> OrganComponent;

	
#if WITH_EDITOR

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
	void OnModeChanged(ENBoneMode NewMode);
	void SetAutomaticTransform();
	FVector FindSafeLocation(const FVector& WorldLocation) const;
	
#endif // WITH_EDITOR

	FString GetDebugLabel() const;
	FVector GetMinimumPoint(const FVector& BaseLocation, const FRotator& OffsetRotation, const FVector2D& SocketUnitSize) const;
	FVector GetMaximumPoint(const FVector& BaseLocation, const FRotator& OffsetRotation, const FVector2D& SocketUnitSize) const;
	
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;
	
private:
	N_WORLD_ICON_HEADER()
};
	