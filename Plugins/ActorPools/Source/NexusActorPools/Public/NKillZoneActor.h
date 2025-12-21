// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NKillZoneComponent.h"
#include "NKillZoneActor.generated.h"

/**
 * A specialized AActor base-class designed as a killzone for actors.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/killzone-volume/">ANKillZoneVolume</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "KillZone Actor")
class NEXUSACTORPOOLS_API ANKillZoneActor : public AActor
{
	GENERATED_BODY()
	
	explicit ANKillZoneActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
	{
		KillZoneComponent = CreateDefaultSubobject<UNKillZoneComponent>(TEXT("KillZone"));
		SetRootComponent(KillZoneComponent);
		RootComponent->Mobility = EComponentMobility::Static;
	}
	
protected:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNKillZoneComponent> KillZoneComponent;
};