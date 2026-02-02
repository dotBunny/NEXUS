// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolSubsystem.h"
#include "INActorPoolItem.h"
#include "NActorPool.h"
#include "NActorPoolSet.h"
#include "NActorPoolSpawnerComponent.h"

namespace NEXUS::ActorPools::ConsoleCommands
{
	static bool bTrackStats = false;
	static FAutoConsoleVariableRef CVAR_bTrackStats(
		TEXT("N.ActorPools.TrackStats"),
		bTrackStats,
		TEXT("Track stats via internal stats system."),
		ECVF_Default | ECVF_Preview);
}

void UNActorPoolSubsystem::Deinitialize()
{
	for (TPair<UClass*, TUniquePtr<FNActorPool>>& Pool : ActorPools)
	{
		Pool.Value->Clear();
		Pool.Value.Reset();
	}
	Super::Deinitialize();
}

void UNActorPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UNActorPoolsSettings* Settings = UNActorPoolsSettings::Get();
	
	UnknownBehaviour = Settings->UnknownBehaviour;
	
	// Check if we have anything to automatically load
	if (Settings->AlwaysCreateSets.Num() == 0) return;
	
	// Check if we have any prefixes to ignore
	if (Settings->IgnoreWorldPrefixes.Num() > 0)
	{
		bool bShouldIgnoreLevel = false;
		const FString WorldName = this->GetWorld()->GetName();
		for (auto& LevelPrefix : Settings->IgnoreWorldPrefixes)
		{
			if (WorldName.StartsWith(LevelPrefix))
			{
				UE_LOG(LogNexusActorPools, Verbose, TEXT("The UWorld(%s) name matches the ignore prefix (%s); skipping UNActorPoolSets set to always create."), *WorldName, *LevelPrefix);
				bShouldIgnoreLevel = true;
			}
		}
		if (bShouldIgnoreLevel) return;
	}
	
	for (auto& Set : Settings->AlwaysCreateSets)
	{
		ApplyActorPoolSet(Set.Get());
	}
}

bool UNActorPoolSubsystem::IsTickable() const
{
	return bHasTickableActorPools || bHasTickableSpawners || NEXUS::ActorPools::ConsoleCommands::bTrackStats;
}

void UNActorPoolSubsystem::Tick(float DeltaTime)
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_ActorPoolSubsystemTick, NEXUS::ActorPools::ConsoleCommands::bTrackStats);
	
	if (NEXUS::ActorPools::ConsoleCommands::bTrackStats)
	{
		SET_DWORD_STAT(STAT_ActorPoolCount, ActorPools.Num())
		SET_DWORD_STAT(STAT_InActors, 0)
		SET_DWORD_STAT(STAT_OutActors, 0)
	}

	if (bHasTickableActorPools)
	{
		for (FNActorPool* Pool : TickableActorPools)
		{
			Pool->Tick();
		}
	}

	if (bHasTickableSpawners)
	{
		for (UNActorPoolSpawnerComponent* Spawner : TickableSpawners)
		{
			Spawner->TickComponent(DeltaTime, LEVELTICK_All, nullptr);
		}
	}

	if (NEXUS::ActorPools::ConsoleCommands::bTrackStats)
	{
		for ( auto Pair = ActorPools.CreateConstIterator(); Pair; ++Pair )
		{
			INC_DWORD_STAT_BY(STAT_InActors, Pair->Value->GetInCount())
			INC_DWORD_STAT_BY(STAT_OutActors, Pair->Value->GetOutCount())
		}
	};
}

bool UNActorPoolSubsystem::CreateActorPool(TSubclassOf<AActor> ActorClass, const FNActorPoolSettings Settings)
{
	if (ActorClass == nullptr)
	{
		UE_LOG(LogNexusActorPools, Warning, TEXT("Unable to create FNActorPool for NULL AActor class."));
		return false;
	}
	
	if (!ActorPools.Contains(ActorClass))
	{
		const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass, Settings));
		UE_LOG(LogNexusActorPools, Verbose, TEXT("Creating a new FNActorPool(%s) in UWorld(%s), raising the total pool count to %i."), *ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
		OnActorPoolAdded.Broadcast(NewPool.Get());
		return true;
	}
	return false;
}

void UNActorPoolSubsystem::ApplyActorPoolSet(UNActorPoolSet* ActorPoolSet)
{
	if (ActorPoolSet->NestedSets.Num() == 0)
	{
		// Optimized fast-path for if the APS is not using nested sets.
		for (const FNActorPoolDefinition& Definition : ActorPoolSet->ActorPools)
		{
			if (ActorPools.Contains(Definition.ActorClass))
			{
				UE_LOG(LogNexusActorPools, Log, TEXT("Attempting to create a new FNActorPool(%s) via UNActorPoolSet that already exists; ignored."), *Definition.ActorClass->GetName());
			}
			else
			{
				CreateActorPool(Definition.ActorClass, Definition.Settings);
			}			
		}
		return;
	}

	// We are going to evaluate and ensure that we are only operating on unique actor pool sets
	TArray<UNActorPoolSet*> OutActorPoolSets;
	if (ActorPoolSet->TryGetUniqueSets(OutActorPoolSets))
	{
		for (const UNActorPoolSet* Set : OutActorPoolSets)
		{
			for (const FNActorPoolDefinition& Definition : Set->ActorPools)
			{
				if (ActorPools.Contains(Definition.ActorClass))
				{
					UE_LOG(LogNexusActorPools, Log, TEXT("Attempting to create a new FNActorPool(%s) via a nested UNActorPoolSet that already exists; ignored."),
						*Definition.ActorClass->GetName());
				}
				else
				{
					CreateActorPool(Definition.ActorClass, Definition.Settings);
				}			
			}
		}
	}
	
}

TArray<FNActorPool*> UNActorPoolSubsystem::GetAllPools() const
{
	TArray<FNActorPool*> ReturnPools;
	ReturnPools.Reserve(ActorPools.Num());
		
	for ( auto Pair = ActorPools.CreateConstIterator(); Pair; ++Pair )
	{
		ReturnPools.Add(Pair.Value().Get());
	}
	return MoveTemp(ReturnPools);
}

void UNActorPoolSubsystem::AddTickableActorPool(FNActorPool* ActorPool)
{
	// Don't add a stub pool to be ticked.
	if (ActorPool->IsStubMode()) return;
	
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
	UClass* ActorClass = Actor->GetClass();
	if (ActorPools.Contains(ActorClass))
	{
		return (*ActorPools.Find(ActorClass))->Return(Actor);
	}
	
	switch (UnknownBehaviour)
	{
		using enum ENActorPoolUnknownBehaviour;
		case CreateDefaultPool:
		{
			UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool via ReturnActor for %s (%s), raising the total pool count to %i."),
				*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
			const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass));
			OnActorPoolAdded.Broadcast(NewPool.Get());
			if (NewPool->DoesSupportInterface())
			{
				INActorPoolItem* ActorItem = Cast<INActorPoolItem>(Actor);
				ActorItem->InitializeActorPoolItem(NewPool.Get());		
			}
			return NewPool->Return(Actor);
		}
		case Ignore:
			return false;
		case Destroy: 
		default:
			if (Actor->IsRooted())
			{
				return false;
			}
			return Actor->Destroy();
	}
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