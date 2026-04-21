// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoneComponent.h"
#include "NBoneActor.generated.h"

/**
 * Standalone bone actor — a UNBoneComponent wrapped in a static actor so it can be dropped
 * directly into a level without being attached to an ANOrganVolume.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Bone Actor")
class NEXUSPROCGEN_API ANBoneActor : public AActor
{
	GENERATED_BODY()

	explicit ANBoneActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
	{
		PrimaryActorTick.bCanEverTick = false;
		PrimaryActorTick.bStartWithTickEnabled = false;

		BoneComponent = CreateDefaultSubobject<UNBoneComponent>(TEXT("NBoneComponent"));
		SetRootComponent(BoneComponent);

		RootComponent->Mobility = EComponentMobility::Static;
	};

public:
	/** The bone component that defines this actor's spatial skeleton contribution. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNBoneComponent> BoneComponent;
};