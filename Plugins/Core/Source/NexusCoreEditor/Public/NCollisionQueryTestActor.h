// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionQueryTestActor.generated.h"

class UCameraComponent;
class USpotLightComponent;

UENUM(BlueprintType)
enum class ECollisionQueryMethod : uint8
{
	LineTrace,
	Sweep,
	Overlap
};

UENUM(BlueprintType)
enum class ECollisionQueryType : uint8
{
	Single,
	Multi,
	Test
};

UENUM(BlueprintType)
enum class ECollisionQueryBy : uint8
{
	Channel,
	ObjectType,
	Profile
};


/**
 * Used to test collision queries in a given map.
 */
UCLASS(DisplayName = "NEXUS: Collision Query Test Actor", ClassGroup = "NEXUS",
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, Tags, Actor, Input, DataLayers, LevelInstance, WorldPartition, HLOD, LOD, Rendering))
class NEXUSCOREEDITOR_API ANCollisionQueryTestActor: public AActor
{
	GENERATED_BODY()

	explicit ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<USceneComponent> StartPointComponent;
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<USceneComponent> EndPointComponent;

	UPROPERTY(EditAnywhere, Category = "Collision Query", DisplayName="Method")
	ECollisionQueryMethod QueryMethod = ECollisionQueryMethod::LineTrace;
	
	UPROPERTY(EditAnywhere, Category = "Collision Query", DisplayName="Type")
	ECollisionQueryType QueryType = ECollisionQueryType::Single;
	
	UPROPERTY(EditAnywhere, Category = "Collision Query", DisplayName="By")
	ECollisionQueryBy QueryBy = ECollisionQueryBy::Channel;
	
private:
	void Query();
	
};