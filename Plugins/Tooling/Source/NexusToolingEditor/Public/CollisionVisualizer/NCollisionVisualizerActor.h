// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCollisionVisualizerSceneComponent.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "NCollisionVisualizerActor.generated.h"

/**
 * In-world handle for the collision visualizer — carries the Start/End scene components the
 * author manipulates in the viewport, and ticks to re-issue queries into its owning widget.
 * Managed by UNCollisionVisualizerWidget; user-level deletion is blocked.
 */
UCLASS(DisplayName = "NEXUS | Collision Visualizer Actor", ClassGroup = "NEXUS", NotPlaceable, Hidden, HideDropdown,
	HideCategories=(Activation, Actor, AssetUserData, Collision, Cooking, DataLayers, HLOD, Input, LevelInstance, LOD,
		Navigation, Networking, Physics, Rendering, Replication, Tags,  WorldPartition))
class NEXUSTOOLINGEDITOR_API ANCollisionVisualizerActor: public AActor
{
	friend class UNCollisionVisualizerWidget;

	GENERATED_BODY()

	explicit ANCollisionVisualizerActor(const FObjectInitializer& ObjectInitializer);

	//~AActor
	virtual bool ShouldTickIfViewportsOnly() const override { return bTickInEditor; }
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual bool CanDeleteSelectedActor(FText& OutReason) const override
	{
		OutReason = FText::FromString("This AActor is managed by the NCollisionVisualizer.");
		return false;
	}
#endif // WITH_EDITOR
	//End AActor

public:
	/** @return the scene component that marks the query start point. */
	UNCollisionVisualizerSceneComponent* GetStartComponent() const { return StartPointComponent; }

	/** @return world-space location of the query start point. */
	FVector GetStartPosition() const { return StartPointComponent->GetComponentLocation(); }

	/** @return the scene component that marks the query end point. */
	UNCollisionVisualizerSceneComponent* GetEndComponent() const { return EndPointComponent; }

	/** @return world-space location of the query end point. */
	FVector GetEndPosition() const { return EndPointComponent->GetComponentLocation(); }

	/** @return end-point location expressed relative to the start point. */
	FVector GetRelativeEndPosition() const { return EndPointComponent->GetRelativeLocation(); }

	/** @return world-space rotation applied to sweep/overlap shapes. */
	FQuat GetRotation() const { return StartPointComponent->GetComponentQuat(); }

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNCollisionVisualizerSceneComponent> StartPointComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNCollisionVisualizerSceneComponent> EndPointComponent;
protected:

	/** Enable/disable ticking while Play-In-Editor is running. */
	void SetTickInGame(bool bTick);

	/** Enable/disable ticking while Simulate-In-Editor is running. */
	void SetTickInSimulation(bool bTick);

	/** Enable/disable ticking from the editor viewport (non-PIE, non-SIE). */
	void SetTickInEditor(bool bTick);

	/** Scene-component transform callback; re-pushes start/end state to the widget. */
	void OnTransformChanged(USceneComponent* Component, EUpdateTransformFlags Flags, ETeleportType Teleport);

private:
	/** Back-reference to the widget that owns and drives this actor. */
	UPROPERTY()
	TObjectPtr<UNCollisionVisualizerWidget> Widget;

	bool bTickInEditor = false;
	bool bTickInGame = false;
	bool bTickInSimulation = false;
};