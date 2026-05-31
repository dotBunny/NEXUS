// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSamplesLevelComponents.generated.h"

class UVolumetricCloudComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class UTextRenderComponent;

UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Samples Level Actor Components")
class UNSamplesLevelComponents : public UObject
{
	GENERATED_BODY()
public:	
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY()
	TObjectPtr<UDecalComponent> Brand;
	UPROPERTY()
	TObjectPtr<UTextRenderComponent> DemoName;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Floor;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WallNorth;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WallEast;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WallSouth;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WallWest;
	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;
	UPROPERTY()
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;
	UPROPERTY()
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
	UPROPERTY()
	TObjectPtr<USkyLightComponent> SkyLight;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SkySphere;
	UPROPERTY()
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;
};