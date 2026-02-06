// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCollisionQueryTestActor.generated.h"

UCLASS(DisplayName = "NEXUS: Transient Actor", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSUIEDITOR_API ANCollisionQueryTestActor: public AActor
{
	GENERATED_BODY()
	DECLARE_DELEGATE_OneParam(FOnTickDelegate, float);
	
	explicit ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		PrimaryActorTick.bCanEverTick = true;
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetTickableWhenPaused(true);
		
		StartPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Start Point"));
#if WITH_EDITORONLY_DATA
		StartPointComponent->bVisualizeComponent = true;
#endif
		RootComponent = StartPointComponent;
		
		EndPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("End Point"));
#if WITH_EDITORONLY_DATA
		EndPointComponent->bVisualizeComponent = true;
#endif
		EndPointComponent->AttachToComponent(StartPointComponent, FAttachmentTransformRules::KeepWorldTransform);
		
		bIsEditorOnlyActor = true;
		
		SetFlags(RF_Transient); // Ensure it never gets saved
	}
	
	virtual void Tick(float DeltaSeconds) override
	{
		UE_LOG(LogTemp, Warning, TEXT("Tick"));
		Super::Tick(DeltaSeconds);
		OnTick.ExecuteIfBound(DeltaSeconds);
	}

public:
	
	USceneComponent* GetStartComponent() const { return StartPointComponent; }
	FVector GetStartPosition() const { return StartPointComponent->GetComponentLocation(); }
	USceneComponent* GetEndComponent() const { return EndPointComponent; }
	FVector GetEndPosition() const { return EndPointComponent->GetComponentLocation(); }
	
	FQuat GetRotation() const { return StartPointComponent->GetComponentQuat(); }
	
	
	FOnTickDelegate OnTick;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> StartPointComponent;
	
	UPROPERTY()
	TObjectPtr<USceneComponent> EndPointComponent;
};