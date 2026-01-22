// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class UVolumetricCloudComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class UTextRenderComponent;

class FNSamplesLevelActorComponents
{
public:	
	TObjectPtr<USceneComponent> SceneRoot;
	TObjectPtr<UDecalComponent> Brand;
	TObjectPtr<UTextRenderComponent> DemoName;
	TObjectPtr<UStaticMeshComponent> Floor;
	TObjectPtr<UStaticMeshComponent> WallNorth;
	TObjectPtr<UStaticMeshComponent> WallEast;
	TObjectPtr<UStaticMeshComponent> WallSouth;
	TObjectPtr<UStaticMeshComponent> WallWest;
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
	TObjectPtr<USkyLightComponent> SkyLight;
	TObjectPtr<UStaticMeshComponent> SkySphere;
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;
};