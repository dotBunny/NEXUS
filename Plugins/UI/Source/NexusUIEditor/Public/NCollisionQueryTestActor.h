// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCollisionQueryTestActor.generated.h"

class UNCollisionQueryTestWidget;

UCLASS(DisplayName = "NEXUS: Transient Actor", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSUIEDITOR_API ANCollisionQueryTestActor: public AActor
{
	friend class UNCollisionQueryTestWidget;
	
	GENERATED_BODY()
	
	explicit ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaSeconds) override;

public:
	
	USceneComponent* GetStartComponent() const { return StartPointComponent; }
	FVector GetStartPosition() const { return StartPointComponent->GetComponentLocation(); }
	USceneComponent* GetEndComponent() const { return EndPointComponent; }
	FVector GetEndPosition() const { return EndPointComponent->GetComponentLocation(); }
	FQuat GetRotation() const { return StartPointComponent->GetComponentQuat(); }
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> StartPointComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EndPointComponent;

private:
	UPROPERTY()
	TObjectPtr<UNCollisionQueryTestWidget> Widget;
};