// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NTransientActor.generated.h"

UCLASS(DisplayName = "NEXUS: Transient Actor", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSUIEDITOR_API ANTransientActor: public AActor
{
	GENERATED_BODY()
	DECLARE_DELEGATE_OneParam(FOnTickDelegate, float);
	
	explicit ANTransientActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		PrimaryActorTick.bCanEverTick = true;
		PrimaryActorTick.bStartWithTickEnabled = true;
		
		SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
		RootComponent = SceneComponent;
		
		bIsEditorOnlyActor = true;
		bIsEditorPreviewActor = true;
		
		SetFlags(RF_Transient); // Ensure it never gets saved
	}
	
	virtual void Tick(float DeltaSeconds) override
	{
		Super::Tick(DeltaSeconds);
		OnTick.ExecuteIfBound(DeltaSeconds);
	}

public:
	FOnTickDelegate OnTick;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneComponent;
};