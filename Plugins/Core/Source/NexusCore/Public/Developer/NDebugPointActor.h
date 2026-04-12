// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDebugPointActor.generated.h"

UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Debug Point Actor", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input, LevelInstance, WorldPartition, DataLayers, Rendering, LOD, HLOD, Physics,
	Collision, Networking, Replication))
class NEXUSCORE_API ANDebugPointActor : public AActor
{
	GENERATED_BODY()
    
public:	
	ANDebugPointActor();
	
	
	static ANDebugPointActor* CreateInstance(UWorld* World, const FVector& Position, const FRotator& Rotation, const FString& Label);
	
	
protected:
	
	UPROPERTY(VisibleAnywhere)
	FString Message;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SphereMesh;
};