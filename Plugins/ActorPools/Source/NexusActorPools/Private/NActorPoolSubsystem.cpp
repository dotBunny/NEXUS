// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolSubsystem.h"
#include "INActorPoolItem.h"
#include "NActorPool.h"
#include "NActorPoolSet.h"
#include "NActorPoolSpawnerComponent.h"
#include "NCoreMinimal.h"

void UNActorPoolSubsystem::Deinitialize()
{
	for (TPair<UClass*, FNActorPool*>& Pool : ActorPools)
	{
		Pool.Value->Clear();
		delete Pool.Value;
	}
	Super::Deinitialize();
}

bool UNActorPoolSubsystem::IsTickable() const
{
	return bHasTickableActorPools || bHasTickableSpawners;
}

void UNActorPoolSubsystem::Tick(float DeltaTime)
{
	for (FNActorPool* Pool : TickableActorPools)
	{
		Pool->Tick();
	}
	for (UNActorPoolSpawnerComponent* Spawner : TickableSpawners)
	{
		Spawner->TickComponent(DeltaTime, LEVELTICK_All, nullptr);
	}
}

bool UNActorPoolSubsystem::CreateActorPool(TSubclassOf<AActor> ActorClass, const FNActorPoolSettings Settings)
{
	if (ActorClass == nullptr)
	{
		N_LOG(Log, TEXT("[NActorPoolSubsystem] Unable to create actor pool for null ActorClass)"));
		return false;
	}
	
	if (!ActorPools.Contains(ActorClass))
	{
		// #RawPointer - I did try to have this as a UObject, however I was not able to resolve behavioral differences
		// with the TSubclassOf<AActor> when looking up pools on UNActorPoolSubsystem.
		FNActorPool* Pool = new FNActorPool(GetWorld(), ActorClass, Settings);
		ActorPools.Add(ActorClass, Pool);
		N_LOG(Log,
			TEXT("[NActorPoolSubsystem] Creating a new pool via CreateActorPool for %s (%s), raising the total pool count to %i."),
			*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		return true;
	}
	return false;
}

void UNActorPoolSubsystem::ApplyActorPoolSet(UNActorPoolSet* ActorPoolSet)
{
	for (const FNActorPoolDefinition& Definition : ActorPoolSet->ActorPools)
	{
		if (ActorPools.Contains(Definition.ActorClass))
		{
			N_LOG(Log, TEXT("[UNActorPoolSubsystem::ApplyActorPoolSet] Attempting to create a new ActorPool via ActorPoolSet that already exists (%s), ignored."),
				*Definition.ActorClass->GetName());
		}
		else
		{
			CreateActorPool(Definition.ActorClass, Definition.Settings);
		}			
	}
}

void UNActorPoolSubsystem::AddTickableActorPool(FNActorPool* ActorPool)
{
	// Don't add a stub pool to be ticked.
	if (ActorPool->GetSettings().HasFlag_ServerOnly() && !GetWorld()->GetAuthGameMode()) return;
	
	TickableActorPools.Add(ActorPool);
	bHasTickableActorPools = (TickableActorPools.Num() > 0);
}

void UNActorPoolSubsystem::RemoveTickableActorPool(FNActorPool* ActorPool)
{
	TickableActorPools.Remove(ActorPool);
	bHasTickableActorPools = (TickableActorPools.Num() > 0);
}

bool UNActorPoolSubsystem::HasTickableActorPool(FNActorPool* ActorPool) const
{
	return TickableActorPools.Contains(ActorPool);
}

void UNActorPoolSubsystem::GetActor(TSubclassOf<AActor> ActorClass, AActor*& ReturnedActor)
{
	ReturnedActor = GetActor<AActor>(ActorClass);
}

void UNActorPoolSubsystem::SpawnActor(TSubclassOf<AActor> ActorClass, FVector Position, FRotator Rotation, AActor*& SpawnedActor)
{
	SpawnedActor = SpawnActor<AActor>(ActorClass, Position, Rotation);
}

bool UNActorPoolSubsystem::ReturnActor(AActor* Actor)
{
	const UClass* ActorClass = Actor->GetClass();
	if (ActorPools.Contains(ActorClass))
	{
		return (*ActorPools.Find(ActorClass))->Return(Actor);
	}
	if (bDestroyUnknownReturnedActors)
	{
		N_LOG(Log, TEXT("[UNActorPoolSubsystem::ReturnActor] Destroying unknown actor sent to pooling system: %s"),
			*Actor->GetPathName());
		return Actor->Destroy();
	}
	return false;
}

void UNActorPoolSubsystem::RegisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent)
{
	TickableSpawners.Add(TargetComponent);
	bHasTickableSpawners = (TickableSpawners.Num() > 0);
}

void UNActorPoolSubsystem::UnregisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent)
{
	TickableSpawners.Remove(TargetComponent);
	bHasTickableSpawners = (TickableSpawners.Num() > 0);
}

void UNActorPoolSubsystem::SetDestroyUnknownReturnedActors(const bool ShouldDestroy)
{
	bDestroyUnknownReturnedActors = ShouldDestroy;
}
