// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NKillZoneComponent.h"
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "INActorPoolItem.h"
#include "NActorPoolSubsystem.h"
#include "NCoreMinimal.h"

UNKillZoneComponent::UNKillZoneComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(false);
	
	SetRelativeLocation(FVector(254, 0, 17), false);
	SetRelativeScale3D(FVector(2.25f, 2.75f, 2.f));
	InitBoxExtent(FVector(100, 100, 5));
	UPrimitiveComponent::SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void UNKillZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	ActorPoolSubsystem = UNActorPoolSubsystem::Get(GetWorld());
	OnComponentBeginOverlap.AddDynamic(this, &UNKillZoneComponent::OnOverlapBegin);
}

void UNKillZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnComponentBeginOverlap.RemoveDynamic(this, &UNKillZoneComponent::OnOverlapBegin);
}

void UNKillZoneComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (bIgnoreStaticActors && !OtherActor->IsRootComponentMovable()) return;

	// Check if actor pool, return to pool
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
	if (ActorPoolSubsystem->HasActorPool(OtherActor->GetClass()))
	{
		ActorPoolSubsystem->ReturnActor(OtherActor);
		KillCount++;
		return;
	}

	if (bIgnoreNonInterfacedActors)
	{
		return;
	}
	
	// Destroy, I guess.
	if (!OtherActor->IsRooted())
	{
		OtherActor->Destroy();
		KillCount++;
	}
	else
	{
		N_LOG_WARNING("Attempted to destroy a root set Actor (%s) via the UNKillZoneComponent, not going to happen.", *OtherActor->GetName())
	}
}
