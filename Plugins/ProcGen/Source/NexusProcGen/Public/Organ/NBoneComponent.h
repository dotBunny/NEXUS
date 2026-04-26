// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellJunctionDetails.h"
#include "Macros/NActorMacros.h"
#include "Types/NCardinalRotation.h"
#include "NBoneComponent.generated.h"

class UNOrganComponent;

/**
 * How a bone's transform is managed in-editor.
 */
UENUM(BlueprintType)
enum class ENBoneMode : uint8
{
	Manual = 0,
	Automatic = 1,
	Disabled = 2
};


/**
 * Scene component that acts as a junction anchor inside an organ.
 *
 * A bone marks a spot where the ProcGen pipeline should emit a cell-to-cell junction — its
 * transform, socket size, and type/requirements determine what fits there. In Automatic mode
 * the editor snaps the bone onto a safe location inside the owning organ.
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Bone", meta=(BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNBoneComponent : public USceneComponent
{
	GENERATED_BODY()

	explicit UNBoneComponent(const FObjectInitializer& ObjectInitializer);

public:

	/** Width/height of the junction socket this bone anchors, measured in grid units. */
	UPROPERTY(EditInstanceOnly, Category = "Bone Component")
	FIntVector2 SocketSize = FIntVector2(4, 4);

	/** Which junction connectivity pattern applies at this bone. */
	UPROPERTY(EditInstanceOnly, Category = "Bone Component")
	ENCellJunctionType Type = ENCellJunctionType::TwoWaySocket;

	/** Whether the bone's junction must be filled, may be blocked, or is optional. */
	UPROPERTY(EditInstanceOnly, Category = "Bone Component")
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::Required;

	/** How the bone's transform is managed (automatic snap, manual placement, or disabled). */
	UPROPERTY(EditAnywhere, Category = "Bone Component")
	ENBoneMode Mode = ENBoneMode::Automatic;

	/** Organ component this bone contributes to; populated on registration. */
	UPROPERTY()
	TObjectPtr<UNOrganComponent> OrganComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Bone Component")
	FGuid Identifier = FGuid::NewGuid();

	//~USceneComponent
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//End USceneComponent

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Editor hook: re-snaps the bone when its transform changes. */
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

	/** Editor hook: reacts to the user flipping the mode between Manual/Automatic/Disabled. */
	void OnModeChanged(ENBoneMode NewMode);

	/** Editor-only: recomputes the bone's transform under Automatic mode. */
	void SetAutomaticTransform();

	/** @return The nearest location inside the owning organ that can safely host this bone. */
	FVector FindSafeLocation(const FVector& WorldLocation) const;
#endif // WITH_EDITOR

	/** @return Human-readable debug label drawn in the viewport overlay. */
	FString GetDebugLabel() const;

	/**
	 * Compute the world-space corner points of the bone's junction socket.
	 * @param SocketUnitSize Socket size in grid units used when deriving the corners.
	 * @return Corner positions in world space, in a consistent order per junction type.
	 */
	TArray<FVector> GetCornerPoints(const FVector2D& SocketUnitSize) const;

	/** Render the bone's debug outline and socket extents via the provided primitive draw interface. */
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;

private:
	N_WORLD_ICON_HEADER()
};
	