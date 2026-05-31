// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "NCollisionVisualizerSceneComponent.generated.h"

/**
 * Start/end gizmo component placed on the collision visualizer actor. Carries a viewport sprite
 * so the author can grab it in-level; bIsEnd selects which sprite resource is displayed.
 */
UCLASS(DisplayName = "NEXUS | Collision Visualizer", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD,
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSTOOLINGEDITOR_API UNCollisionVisualizerSceneComponent: public USceneComponent
{
	friend class ANCollisionVisualizerActor;

	GENERATED_BODY()

	//~USceneComponent
	virtual void OnRegister() override;
	//End USceneComponent

#if WITH_EDITOR
	/** Attach the editor-only billboard sprite used to make this gizmo selectable in the viewport. */
	void CreateCustomSpriteComponent();
#endif // WITH_EDITOR

	const FString StartSpritePath = TEXT("/NexusTooling/EditorResources/S_NCollisionVisualizerSceneComponentStart.S_NCollisionVisualizerSceneComponentStart");
	const FString EndSpritePath = TEXT("/NexusTooling/EditorResources/S_NCollisionVisualizerSceneComponentEnd.S_NCollisionVisualizerSceneComponentEnd");

	/** When true this component represents the query's end point and uses the "End" sprite. */
	bool bIsEnd = false;
};