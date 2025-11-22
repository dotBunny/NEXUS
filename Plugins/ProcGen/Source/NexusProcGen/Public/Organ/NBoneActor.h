// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoneComponent.h"
#include "NBoneActor.generated.h"

UCLASS(DisplayName = "NBone Actor")
class NEXUSPROCGEN_API ANBoneActor : public AActor
{
	GENERATED_BODY()
	
	explicit ANBoneActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
	{
		PrimaryActorTick.bCanEverTick = false;
		PrimaryActorTick.bStartWithTickEnabled = false;
		
		// Add our root component
		BoneComponent = CreateDefaultSubobject<UNBoneComponent>(TEXT("NBoneComponent"));
		SetRootComponent(BoneComponent);
	
		RootComponent->Mobility = EComponentMobility::Static;
	};
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNBoneComponent> BoneComponent;
};