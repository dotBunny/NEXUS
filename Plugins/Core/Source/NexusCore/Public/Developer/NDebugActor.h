// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "GameFramework/Actor.h"
#include "NDebugActor.generated.h"

/**
 * A disposable diagnostic actor used to visualize a world-space location or arbitrary geometry during development.
 *
 * ANDebugActor is intentionally hidden from the editor's Place Actors panel; it is spawned at runtime via
 * CreateInstance when a call site wants a tangible, clickable marker backed by a sphere mesh and a human-readable
 * label. It carries two visual components — a default sphere (SphereMesh) for point-style markers and a
 * UDynamicMeshComponent (DynamicMesh) for richer geometry overrides via OverrideWithDynamicMesh. It has no
 * gameplay behavior and should not be shipped in release content.
 * @remark Spawn via CreateInstance from debug/test code, not from gameplay logic.
 */
UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Debug Actor", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input, LevelInstance, WorldPartition, DataLayers, Rendering, LOD, HLOD, Physics,
	Collision, Networking, Replication))
class NEXUSCORE_API ANDebugActor : public AActor
{
	GENERATED_BODY()

public:
	
	ANDebugActor();

	/** Human-readable message stored with actor instance. */
	UPROPERTY(EditAnywhere)
	FString Message;
	
	/**
	 * Spawns a debug actor in the supplied world at the given transform, labeled for quick identification.
	 * @param World The world to spawn into.
	 * @param Position World position of the spawned marker.
	 * @param Rotation World rotation of the spawned marker.
	 * @param Label Human-readable message stored on the actor and shown in the details panel.
	 * @param Scale World scale applied to the spawned marker.
	 * @return The newly spawned debug actor, or nullptr if spawning failed.
	 */
	static ANDebugActor* CreateInstance(UWorld* World, const FVector& Position, const FRotator& Rotation, const FString& Label,
		const FVector& Scale = FVector::OneVector);

	/**
	 * Accessor for the sphere marker component.
	 * @return The static-mesh component used as the default point-style marker.
	 */
	TObjectPtr<UStaticMeshComponent> GetStaticMeshComponent() const { return SphereMesh; }

	/**
	 * Removes this actor from the editor's World Outliner / Scene Hierarchy panel without affecting viewport
	 * visibility, selection, or any other behavior. Compiled out of non-editor builds.
	 */
	void HideInSceneOutliner()
	{
#if WITH_EDITORONLY_DATA
		bListedInSceneOutliner = false;
#endif // WITH_EDITORONLY_DATA
	}

	/**
	 * Restores this actor's visibility in the editor's World Outliner / Scene Hierarchy panel after a prior
	 * HideInSceneOutliner call. Compiled out of non-editor builds.
	 */
	void ShowInSceneOutliner()
	{
#if WITH_EDITORONLY_DATA
		bListedInSceneOutliner = true;
#endif // WITH_EDITORONLY_DATA
	}
	
	/**
	 * Replaces the sphere marker with arbitrary dynamic-mesh geometry, hiding the sphere in the process.
	 * Calls Modify() on the dynamic-mesh component so the change is captured by the editor's transaction system.
	 * @param NewMesh Mesh data to install. Moved into the component.
	 * @param Material Material assigned to slot 0 of the dynamic mesh. May be null.
	 */
	void OverrideWithDynamicMesh(FDynamicMesh3 NewMesh, UMaterialInterface* Material) const
	{
		SphereMesh->SetVisibility(false, false);
		DynamicMesh->Modify();
		DynamicMesh->SetMesh(MoveTemp(NewMesh));
		DynamicMesh->SetMaterial(0, Material);
	}

	/**
	 * Accessor for the dynamic-mesh component used by OverrideWithDynamicMesh.
	 * @return The dynamic-mesh component that holds any overridden geometry.
	 */
	TObjectPtr<UDynamicMeshComponent> GetDynamicMeshComponent() const { return DynamicMesh; }

protected:
	/** Sphere mesh component used as the visible marker in the viewport. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SphereMesh;

	/** Dynamic-mesh component used when OverrideWithDynamicMesh installs custom geometry in place of the sphere. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDynamicMeshComponent> DynamicMesh;
};