// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDebugActor.generated.h"

/**
 * A disposable diagnostic actor used to visualize a world-space location during development.
 *
 * ANDebugActor is intentionally hidden from the editor's Place Actors panel; it is spawned at runtime
 * via CreateInstance when a call site wants a tangible, clickable marker backed by a sphere mesh and a
 * human-readable label. It has no gameplay behavior and should not be shipped in release content.
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

protected:
	/** Human-readable message displayed alongside the actor for quick identification. */
	UPROPERTY(VisibleAnywhere)
	FString Message;

	/** Sphere mesh component used as the visible marker in the viewport. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SphereMesh;
};