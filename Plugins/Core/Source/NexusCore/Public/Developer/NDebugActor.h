// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDebugActor.generated.h"

UCLASS(NotPlaceable, HideDropdown, Hidden, ClassGroup = "NEXUS", DisplayName = "NEXUS | Debug Actor", HideCategories=(Tags, Activation, Cooking,
	AssetUserData, Navigation, Actor, Input, LevelInstance, WorldPartition, DataLayers, Rendering, LOD, HLOD, Physics,
	Collision, Networking, Replication))
class NEXUSCORE_API ANDebugActor : public AActor
{
	GENERATED_BODY()
    
public:	
	ANDebugActor();
	
	
	static ANDebugActor* CreateInstance(UWorld* World, const FVector& Position, const FRotator& Rotation, const FString& Label, 
		const FVector& Scale = FVector::OneVector);

protected:
	
	UPROPERTY(VisibleAnywhere)
	FString Message;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SphereMesh;
};