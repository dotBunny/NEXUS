// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NKillZoneComponent.h"
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "INActorPoolItem.h"
#include "NActorPoolSubsystem.h"

namespace NEXUS::ActorPools::KillZone
{
	inline const FVector DefaultRelativeLocation = FVector(254, 0, 17);
	inline const FVector DefaultRelativeScale = FVector(2.25f, 2.75f, 2.f);
	inline const FVector DefaultBoxExtents = FVector(100,100,5);
}

UNKillZoneComponent::UNKillZoneComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(false);
	
	SetRelativeLocation(NEXUS::ActorPools::KillZone::DefaultRelativeLocation, false);
	SetRelativeScale3D(NEXUS::ActorPools::KillZone::DefaultRelativeScale);
	InitBoxExtent(NEXUS::ActorPools::KillZone::DefaultBoxExtents);
	UPrimitiveComponent::SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void UNKillZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	ActorPoolSubsystem = UNActorPoolSubsystem::Get(World);

	WorldFallDamageType = GetDefault<UDamageType>();
	AWorldSettings* WorldSettings = World->GetWorldSettings( true );
	if ( WorldSettings && WorldSettings->KillZDamageType )
	{
		WorldFallDamageType = WorldSettings->KillZDamageType->GetDefaultObject<UDamageType>();
	}

	OnComponentBeginOverlap.AddDynamic(this, &UNKillZoneComponent::OnOverlapBegin);
}

void UNKillZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnComponentBeginOverlap.RemoveDynamic(this, &UNKillZoneComponent::OnOverlapBegin);
}

void UNKillZoneComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	// If there's no root object it's not moving so were considering it static
	if (bIgnoreStaticActors && (OtherActor->GetRootComponent() == nullptr || !OtherActor->IsRootComponentMovable())) return;
	
	// Check if in an actor pool, return to the pool
	INActorPoolItem* ActorItem = Cast<INActorPoolItem>(OtherActor);
	if (ActorItem != nullptr)
	{
		if (!ActorItem->ReturnToActorPool())
		{
			if (!ActorItem->GetActorPoolSettings().HasFlag_ServerOnly())
			{
				// The intent is still to destroy
				OtherActor->Destroy();
			}
			KillCount++;
			return;
		}

		// Normal return happened via return to actor pool
		KillCount++;
		return;
	}

	// Check if we have a pool for this Actor, but it just doesn't implement the interface
	if (ActorPoolSubsystem != nullptr && 
		ActorPoolSubsystem->HasActorPool(OtherActor->GetClass()))
	{
		ActorPoolSubsystem->ReturnActor(OtherActor);
		KillCount++;
		return; 
	}

	
	switch (UnknownBehaviour)
	{
	case ENKillZoneBehavior::Ignore:
		break;
	case ENKillZoneBehavior::ApplyFellOutOfWorld:
		if (!OtherActor->IsRooted())
		{
			OtherActor->FellOutOfWorld(*WorldFallDamageType);
			KillCount++;
		}
		else
		{
			UE_LOG(LogNexusActorPools, Warning, TEXT("UNKillZoneComponent unable to apply FellOutOfWorld to rooted unknown AActor(%s)."), *OtherActor->GetName());
		}
		break;
	case ENKillZoneBehavior::ReturnToActorPool:
		if (ActorPoolSubsystem != nullptr &&
		ActorPoolSubsystem->ReturnActor(OtherActor))
		{
			KillCount++;
		}
		else
		{
			UE_LOG(LogNexusActorPools, Warning, TEXT("Unable to return the unknown AActor(%s) to an Actor Pool."), *OtherActor->GetName());
		}
		break;
	}
}
