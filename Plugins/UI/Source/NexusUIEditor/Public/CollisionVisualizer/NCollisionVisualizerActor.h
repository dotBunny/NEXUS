// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "NCollisionVisualizerActor.generated.h"

UCLASS(DisplayName = "NEXUS: Collision Visualizer Actor", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD, 
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSUIEDITOR_API ANCollisionVisualizerActor: public AActor
{
	friend class UNCollisionVisualizerWidget;
	
	GENERATED_BODY()
	
	explicit ANCollisionVisualizerActor(const FObjectInitializer& ObjectInitializer);
	virtual bool ShouldTickIfViewportsOnly() const override { return bTickInEditor; }
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual bool CanDeleteSelectedActor(FText& OutReason) const override
	{
		OutReason = FText::FromString("This AActor is managed by the NCollisionVisualizer.");
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
	
	void SetTickInGame(bool bTick);
	void SetTickInSimulation(bool bTick);
	void SetTickInEditor(bool bTick);
	
	void OnTransformChanged(USceneComponent* Component, EUpdateTransformFlags Flags, ETeleportType Teleport);
	
private:
	UPROPERTY()
	TObjectPtr<UNCollisionVisualizerWidget> Widget;
	
	bool bTickInEditor = false;
	bool bTickInGame = false;
	bool bTickInSimulation = false;
};