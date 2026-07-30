// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolSubsystem.h"
#include "INActorPoolItem.h"
#include "NActorPool.h"
#include "NActorPoolSet.h"
#include "NActorPoolSpawnerComponent.h"
#include "Macros/NValidationMacros.h"

namespace NEXUS::ActorPools::ConsoleCommands
{
	static bool bTrackStats = false;
	static FAutoConsoleVariableRef CVAR_bTrackStats(
		TEXT("N.ActorPools.TrackStats"),
		bTrackStats,
		TEXT("Track stats via internal stats system."),
		ECVF_Default | ECVF_Preview);
}

void UNActorPoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	const UNActorPoolsSettings* Settings = UNActorPoolsSettings::Get();
	UnknownBehavior = Settings->UnknownBehavior;

	// Check if we have anything to automatically load
	if (Settings->AlwaysCreateSets.IsEmpty())
	{
		Super::OnWorldBeginPlay(InWorld);
		return;
	}

	// Check if we have any prefixes to ignore
	if (!Settings->IgnoreWorldPrefixes.IsEmpty())
	{
		const FString WorldName = InWorld.GetName();
		for (const FString& LevelPrefix : Settings->IgnoreWorldPrefixes)
		{
			if (WorldName.StartsWith(LevelPrefix))
			{
				UE_LOG(LogNexusActorPools, Verbose, TEXT("The UWorld(%s) name matches the ignore prefix (%s); skipping UNActorPoolSets set to always create."), *WorldName, *LevelPrefix);
				Super::OnWorldBeginPlay(InWorld);
				return;
			}
		}
	}

	for (auto& Set : Settings->AlwaysCreateSets)
	{
		// Ensure the set has been loaded
		UNActorPoolSet* LoadedSet = Set.LoadSynchronous();
		if (LoadedSet == nullptr) continue;
		ApplyActorPoolSet(LoadedSet);
	}

	Super::OnWorldBeginPlay(InWorld);
}

void UNActorPoolSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	TickableActorPools.Empty();
	TickableSpawners.Empty();

	for (TPair<UClass*, TUniquePtr<FNActorPool>>& Pool : ActorPools)
	{
		Pool.Value->Clear(); // Releases all actors from pool (not forcibly destroying cause of world teardown)
	}

	// Destroy the pools in one pass via Empty() rather than Reset()-ing each in the loop above. Clear() can
	// re-enter Blueprint (ReleaseActor -> OnReleasedFromActorPool), and a re-entrant GetActorPoolStats() would
	// observe a map entry whose TUniquePtr had already been reset to null and dereference it. Empty() destroys
	// every pool atomically (running ~FNActorPool, which releases the UObject back-pointer), so the map is never
	// left holding a present-but-null entry.
	ActorPools.Empty();

	Super::OnWorldEndPlay(InWorld);
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
		// Reverse index walk so a pool that finishes warming can be removed in-place (RemoveAtSwap) without
		// disturbing iteration. Order is irrelevant for warm-up, so the swap is safe.
		for (int32 Index = TickableActorPools.Num() - 1; Index >= 0; --Index)
		{
			if (!TickableActorPools[Index]->Tick())
			{
				TickableActorPools.RemoveAtSwap(Index, EAllowShrinking::No);
			}
		}
		bHasTickableActorPools = !TickableActorPools.IsEmpty();
	}

	if (bHasTickableSpawners)
	{
		// Reverse mainly so if one of these things unregisters due to some callback we don't implode (ub)
		for (int32 Index = TickableSpawners.Num() - 1; Index >= 0; --Index)
		{
			if (UNActorPoolSpawnerComponent* Spawner = TickableSpawners[Index])
			{
				Spawner->TickSpawner(DeltaTime);
			}
		}
		bHasTickableSpawners = !TickableSpawners.IsEmpty();
	}

	if (NEXUS::ActorPools::ConsoleCommands::bTrackStats)
	{
		for ( auto Pair = ActorPools.CreateConstIterator(); Pair; ++Pair )
		{
			INC_DWORD_STAT_BY(STAT_InActors, Pair->Value->GetAvailableCount())
			INC_DWORD_STAT_BY(STAT_OutActors, Pair->Value->GetSpawnedCount())
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

		UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new FNActorPool(%s) in UWorld(%s), raising the total pool count to %i."),
			*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());

		OnActorPoolAdded.Broadcast(NewPool.Get());
		return true;
	}
	return false;
}

bool UNActorPoolSubsystem::AddDefaultSettings(const TSubclassOf<AActor> ActorClass, const FNActorPoolSettings& Settings)
{
	if (DefaultSettings.Contains(ActorClass))
	{
		return false;
	}
	DefaultSettings.Add(ActorClass, Settings);
	return true;
}

bool UNActorPoolSubsystem::UpdateDefaultSettings(const TSubclassOf<AActor> ActorClass, const FNActorPoolSettings& Settings)
{
	if (DefaultSettings.Contains(ActorClass))
	{
		DefaultSettings[ActorClass] = Settings;
		return true;
	}
	return false;
}

bool UNActorPoolSubsystem::RemoveDefaultSettings(const TSubclassOf<AActor> ActorClass)
{
	return DefaultSettings.Remove(ActorClass) != 0;
}

bool UNActorPoolSubsystem::HasDefaultSettings(const TSubclassOf<AActor> ActorClass) const
{
	return DefaultSettings.Contains(ActorClass);
}

FNActorPoolSettings UNActorPoolSubsystem::GetDefaultSettings(const TSubclassOf<AActor> ActorClass) const
{
	if (!DefaultSettings.Contains(ActorClass))
	{
		return UNActorPoolsSettings::Get()->DefaultSettings;
	}
	return DefaultSettings.FindChecked(ActorClass);
}

void UNActorPoolSubsystem::ApplyActorPoolSet(UNActorPoolSet* ActorPoolSet)
{
	N_VALIDATE_RETURN_VOID(LogNexusActorPools, ActorPoolSet);

	if (ActorPoolSet->NestedSets.IsEmpty())
	{
		// Optimized fast-path for if the APS is not using nested sets.
		for (const FNActorPoolDefinition& Definition : ActorPoolSet->ActorPools)
		{
			if (ActorPools.Contains(Definition.ActorClass))
			{
				UE_LOG(LogNexusActorPools, Verbose,
					TEXT("Attempting to create a new FNActorPool(%s) via UNActorPoolSet that already exists; ignored."),
					*Definition.ActorClass->GetName());
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
	ActorPoolSet->GetUniqueSets(OutActorPoolSets);
	for (const UNActorPoolSet* Set : OutActorPoolSets)
	{
		for (const FNActorPoolDefinition& Definition : Set->ActorPools)
		{
			if (ActorPools.Contains(Definition.ActorClass))
			{
				UE_LOG(LogNexusActorPools, Verbose,
					TEXT("Attempting to create a new FNActorPool(%s) via a nested UNActorPoolSet that already exists; ignored."),
					*Definition.ActorClass->GetName());
			}
			else
			{
				CreateActorPool(Definition.ActorClass, Definition.Settings);
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
		if (FNActorPool* Pool = Pair.Value().Get())
		{
			ReturnPools.Add(Pool);
		}
	}
	return ReturnPools;
}

void UNActorPoolSubsystem::AddTickableActorPool(FNActorPool* ActorPool)
{
	// Don't add a stub pool to be ticked.
	if (ActorPool->IsStubMode()) return;

	// Ensure we only ever add a pool once
	TickableActorPools.AddUnique(ActorPool);
	bHasTickableActorPools = !TickableActorPools.IsEmpty();
}

void UNActorPoolSubsystem::RemoveTickableActorPool(FNActorPool* ActorPool)
{
	TickableActorPools.Remove(ActorPool);
	bHasTickableActorPools = !TickableActorPools.IsEmpty();
}

bool UNActorPoolSubsystem::HasTickableActorPool(FNActorPool* ActorPool) const
{
	return TickableActorPools.Contains(ActorPool);
}

bool UNActorPoolSubsystem::GetActor(TSubclassOf<AActor> ActorClass, AActor*& ReturnedActor)
{
	ReturnedActor = nullptr;
	N_VALIDATE_RETURN(LogNexusActorPools, ActorClass, false);
	ReturnedActor = GetActor<AActor>(ActorClass);
	return ReturnedActor != nullptr;
}

bool UNActorPoolSubsystem::SpawnActor(TSubclassOf<AActor> ActorClass, FVector Position, FRotator Rotation, AActor*& SpawnedActor)
{
	SpawnedActor = nullptr;
	N_VALIDATE_RETURN(LogNexusActorPools, ActorClass, false);
	SpawnedActor = SpawnActor<AActor>(ActorClass, Position, Rotation);
	return SpawnedActor != nullptr;
}

bool UNActorPoolSubsystem::ReturnActor(AActor* Actor)
{
	N_VALIDATE_RETURN(LogNexusActorPools, Actor, false);

	UClass* ActorClass = Actor->GetClass();
	// Hot path: single lookup, branch on the result rather than Contains() + Find().
	if (const TUniquePtr<FNActorPool>* Existing = ActorPools.Find(ActorClass))
	{
		return (*Existing)->Return(Actor);
	}

	switch (UnknownBehavior)
	{
		using enum ENActorPoolUnknownBehavior;
		case CreateDefaultPool:
		{
			if (HasDefaultSettings(ActorClass))
			{
				UE_LOG(LogNexusActorPools, Log,
					TEXT("Creating a new pool via ReturnActor for %s (%s) using the registered default settings, raising the total pool count to %i."),
					*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());

				// Presence was just verified, so pass the map entry straight to the constructor rather than
				// GetDefaultSettings(), which would return the ~176-byte struct by value (an extra copy).
				const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass, DefaultSettings.FindChecked(ActorClass)));
				OnActorPoolAdded.Broadcast(NewPool.Get());
				if (NewPool->ImplementsPoolItemInterface())
				{
					INActorPoolItem* ActorItem = Cast<INActorPoolItem>(Actor);
					ActorItem->InitializeActorPoolItem(NewPool.Get());
				}
				return NewPool->Return(Actor);
			}

			UE_LOG(LogNexusActorPools, Log, TEXT("Creating a new pool via ReturnActor for %s (%s), raising the total pool count to %i."),
				*ActorClass->GetName(), *GetWorld()->GetName(), ActorPools.Num());
			const TUniquePtr<FNActorPool>& NewPool = ActorPools.Add(ActorClass, MakeUnique<FNActorPool>(GetWorld(), ActorClass));
			OnActorPoolAdded.Broadcast(NewPool.Get());
			if (NewPool->ImplementsPoolItemInterface())
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
				UE_LOG(LogNexusActorPools, Error, TEXT("Unable to return an AActor with the Destroy behavior that is rooted."));
				return false;
			}
			Actor->Destroy();

			// The Actor was handled (destroyed), so report success to callers.
			return true;
	}
}

void UNActorPoolSubsystem::ReturnAllActors()
{
	for ( auto Pair = ActorPools.CreateConstIterator(); Pair; ++Pair )
	{
		FNActorPool* Pool = Pair->Value.Get();
		if (Pool->GetSettings().HasSupportFlag_ReturnAll())
		{
			Pair->Value.Get()->ReturnAll(true);
		}
	}
}

void UNActorPoolSubsystem::RegisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent)
{
	if (TargetComponent == nullptr) return;
	TickableSpawners.AddUnique(TargetComponent);
	bHasTickableSpawners = !TickableSpawners.IsEmpty();
}

void UNActorPoolSubsystem::UnregisterTickableSpawner(UNActorPoolSpawnerComponent* TargetComponent)
{
	if (TargetComponent == nullptr) return;
	TickableSpawners.Remove(TargetComponent);
	bHasTickableSpawners = !TickableSpawners.IsEmpty();
}