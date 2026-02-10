// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "NCollisionQueryTestActor.generated.h"

class UNCollisionQueryTestWidget;

UCLASS(DisplayName = "NEXUS: Collision Query Test Actor", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSUIEDITOR_API ANCollisionQueryTestActor: public AActor
{
	friend class UNCollisionQueryTestWidget;
	friend class UNCollisionQueryTestComponent;
	
	GENERATED_BODY()
	
	explicit ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer);
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual bool CanDeleteSelectedActor(FText& OutReason) const override
	{
		OutReason = FText::FromString("This AActor is managed by the NCollisionQueryTest.");
		return false;
	}
#endif

public:
	USceneComponent* GetStartComponent() const { return StartPointComponent; }
	FVector GetStartPosition() const { return StartPointComponent->GetComponentLocation(); }
	USceneComponent* GetEndComponent() const { return EndPointComponent; }
	FVector GetEndPosition() const { return EndPointComponent->GetComponentLocation(); }
	FVector GetRelativeEndPosition() const { return EndPointComponent->GetRelativeLocation(); }
	FQuat GetRotation() const { return StartPointComponent->GetComponentQuat(); }
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> StartPointComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EndPointComponent;
protected:
	
	void OnTransformChanged(USceneComponent* Component, EUpdateTransformFlags Flags, ETeleportType Teleport);
	
private:
	UPROPERTY()
	TObjectPtr<UNCollisionQueryTestWidget> Widget;
	
	bool bTickInEditor = false;
	bool bTickInGame = false;
	bool bTickInSimulation = false;
};