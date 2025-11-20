// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellJunctionDetails.h"
#include "Types/NCardinalRotation.h"
#include "NBoneComponent.generated.h"


// Bones will be anchors for junctions
// need to add something like a 'organExclusion' ? for the actual space

UCLASS(ClassGroup=(Nexus), meta=(BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSPROCGEN_API UNBoneComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	void OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
#endif // WITH_EDITOR
	
	void DrawDebugPDI(FPrimitiveDrawInterface* PDI) const;
	
	UPROPERTY(EditInstanceOnly)
	FIntVector2 UnitSize = FIntVector2(4, 2);

	UPROPERTY(VisibleAnywhere)
	FNCardinalRotation RootRelativeCardinalRotation;
	
	UPROPERTY(EditInstanceOnly)
	ENCellJunctionType Type = ENCellJunctionType::CJT_TwoWaySocket;

	UPROPERTY(EditInstanceOnly)
	ENCellJunctionRequirements Requirements = ENCellJunctionRequirements::CJR_Required;
};
	