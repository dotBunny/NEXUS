// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "NCollisionVisualizerSceneComponent.generated.h"

UCLASS(DisplayName = "NEXUS: Collision Visualizer Scene Component", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSTOOLINGEDITOR_API UNCollisionVisualizerSceneComponent: public USceneComponent
{
	friend class ANCollisionVisualizerActor;
	
	GENERATED_BODY()
	
	virtual void OnRegister() override;

#if WITH_EDITOR
	void CreateCustomSpriteComponent();
#endif // WITH_EDITOR
	
	const FString StartSpritePath = TEXT("/NexusTooling/EditorResources/S_NCollisionVisualizerSceneComponentStart.S_NCollisionVisualizerSceneComponentStart");
	const FString EndSpritePath = TEXT("/NexusTooling/EditorResources/S_NCollisionVisualizerSceneComponentEnd.S_NCollisionVisualizerSceneComponentEnd");
	bool bIsEnd = false;
};