// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NKillZoneComponent.h"
#include "NKillZoneVolume.generated.h"

/**
 * A specialized AVolume base-class designed as a killzone for actors.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/killzone-volume/">ANKillZoneVolume</a>
 */
UCLASS()
class NEXUSACTORPOOLS_API ANKillZoneVolume : public AVolume
{
	GENERATED_BODY()
	
	explicit ANKillZoneVolume(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
	{
		KillZoneComponent = CreateDefaultSubobject<UNKillZoneComponent>(TEXT("KillZone"));
		RootComponent->Mobility = EComponentMobility::Static;
	}
	
protected:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UNKillZoneComponent> KillZoneComponent;
};