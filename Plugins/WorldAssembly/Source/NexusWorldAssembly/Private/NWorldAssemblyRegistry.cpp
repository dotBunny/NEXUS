// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyRegistry.h"

#include "NWorldAssemblyMinimal.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellLevelInstance.h"
#include "Cell/NCellRootComponent.h"
#include "Organ/NOrganComponent.h"

TArray<UNCellRootComponent*> FNWorldAssemblyRegistry::CellRoots;
TArray<UNBoneComponent*> FNWorldAssemblyRegistry::Bones;
TArray<UNCellJunctionComponent*> FNWorldAssemblyRegistry::CellJunctions;
TArray<UNOrganComponent*> FNWorldAssemblyRegistry::Organs;

TArray<UNAssemblyOperation*> FNWorldAssemblyRegistry::Operations;
FOnAssemblyOperationStateChanged FNWorldAssemblyRegistry::OnOperationStateChanged;
TMap<int32, TArray<ANCellLevelInstance*>> FNWorldAssemblyRegistry::CellLevelInstances;


TArray<UNCellJunctionComponent*> FNWorldAssemblyRegistry::GetCellJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted)
{
	TArray<UNCellJunctionComponent*> JunctionComponents;
	if (Level == nullptr)
	{
		return MoveTemp(JunctionComponents);
	}

	for (UNCellJunctionComponent* Junction : GetCellJunctionComponents())
	{
		if (Junction->ComponentIsInLevel(Level))
		{
			JunctionComponents.Add(Junction);
		}
	}

	if (bSorted)
	{
		JunctionComponents.Sort([](const UNCellJunctionComponent& A, const UNCellJunctionComponent& B)
		{
			return A.Details.InstanceIdentifier < B.Details.InstanceIdentifier;
		});
	}
	
	return MoveTemp(JunctionComponents);
}

TArray<UNOrganComponent*> FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(const ULevel* Level, const bool bSorted)
{
	TArray<UNOrganComponent*> OrganComponents;
	if (Level == nullptr)
	{
		return MoveTemp(OrganComponents);
	}

	for (UNOrganComponent* Organ : GetOrganComponents())
	{
		if (Organ->ComponentIsInLevel(Level))
		{
			OrganComponents.Add(Organ);
		}
	}
	
	if (bSorted)
	{
		OrganComponents.Sort([](const UNOrganComponent& A, const UNOrganComponent& B) {
			return A.Identifier < B.Identifier;
		});
	}
	

	return MoveTemp(OrganComponents);
}

TArray<UNBoneComponent*> FNWorldAssemblyRegistry::GetBoneComponentsFromLevel(const ULevel* Level, const bool bSorted)
{
	TArray<UNBoneComponent*> BoneComponents;
	if (Level == nullptr)
	{
		return MoveTemp(BoneComponents);
	}

	for (UNBoneComponent* Bone : GetBoneComponents())
	{
		if (Bone->ComponentIsInLevel(Level))
		{
			BoneComponents.Add(Bone);
		}
	}
	
	if (bSorted)
	{
		BoneComponents.Sort([](const UNBoneComponent& A, const UNBoneComponent& B) {
			return A.Identifier < B.Identifier;
		});
	}

	return MoveTemp(BoneComponents);
}

UNCellRootComponent* FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(const ULevel* Level)
{
	for (UNCellRootComponent* RootComponent : GetCellRootComponents())
	{
		if (RootComponent->ComponentIsInLevel(Level))
		{
			return RootComponent;
		}
	}
	return nullptr;
}

bool FNWorldAssemblyRegistry::HasBoneComponents()
{
	return !Bones.IsEmpty();
}

bool FNWorldAssemblyRegistry::HasRootComponents()
{
	return !CellRoots.IsEmpty();
}

bool FNWorldAssemblyRegistry::HasJunctionComponents()
{
	return !CellJunctions.IsEmpty();
}

bool FNWorldAssemblyRegistry::HasOrganComponents()
{
	return !Organs.IsEmpty();
}

bool FNWorldAssemblyRegistry::HasOrganComponentsInWorld(const UWorld* World)
{
	for (const UNOrganComponent* Component : GetOrganComponents())
	{
		if (Component->GetWorld() == World) return true;
	}
	return false;
}

bool FNWorldAssemblyRegistry::HasOperations(bool bIgnoreEditorModeOperation)
{
#if WITH_EDITOR	
	if (Operations.Num() == 1 && Operations[0]->GetName() == NEXUS::WorldAssembly::Operations::EditorMode)
	{
		return false;
	}
#endif // WITH_EDITOR
	return !Operations.IsEmpty();
}

bool FNWorldAssemblyRegistry::HasCellLevelInstances(const int32 OperationTicket, const bool bIsLevelLoaded)
{
	if (OperationTicket == 0)
	{
		return !CellLevelInstances.IsEmpty();
	}
	
	if (CellLevelInstances.Contains(OperationTicket))
	{
		return !CellLevelInstances[OperationTicket].IsEmpty();
	}
	return false;
}

TArray<ANCellLevelInstance*> FNWorldAssemblyRegistry::GetCellLevelInstancesInRange(const FVector& Location, const double Range, const bool bIsLevelLoaded, const int32 OperationTicket)
{
	TArray<ANCellLevelInstance*> Results;
	if (CellLevelInstances.IsEmpty()) return MoveTemp(Results);
	
	const double RangeSquared = Range * Range;

	// Distance check method
	auto AppendInRange = [&Results, &Location, RangeSquared, &bIsLevelLoaded](const TArray<ANCellLevelInstance*>& Instances)
	{
		for (ANCellLevelInstance* Instance : Instances)
		{
			if (Instance == nullptr) continue;
			if (bIsLevelLoaded && !Instance->IsLoaded()) continue;
			if (FVector::DistSquared(Location, Instance->GetActorLocation()) <= RangeSquared)
			{
				Results.Add(Instance);
			}
		}
	};

	if (OperationTicket != 0)
	{
		// Specific-only
		const TArray<ANCellLevelInstance*>* LevelInstancePtr = CellLevelInstances.Find(OperationTicket);
		if (LevelInstancePtr != nullptr)
		{
			AppendInRange(*LevelInstancePtr);
		}
	}
	else
	{
		// Search them all
		for (const auto& Pair : CellLevelInstances)
		{
			AppendInRange(Pair.Value);
		}
	}

	return MoveTemp(Results);
}

bool FNWorldAssemblyRegistry::HasCellLevelInstance(const int32 OperationTicket, const FGuid LevelInstanceSpawnGuid, const bool bIsLevelLoaded)
{
	TArray<ANCellLevelInstance*>* LevelInstances = CellLevelInstances.Find(OperationTicket);
	if (LevelInstances != nullptr && LevelInstances->Num() > 0)
	{
		for (ANCellLevelInstance* LevelInstance : *LevelInstances)
		{
			if (LevelInstance->GetLevelInstanceSpawnGuid() == LevelInstanceSpawnGuid)
			{
				if (bIsLevelLoaded && !LevelInstance->IsLoaded()) continue;
				return true;
			}
		}
	}
	return false;
}

bool FNWorldAssemblyRegistry::HasCellLevelInstances(const TArray<FNCellLevelInstanceLocator>& LevelInstances, const bool bIsLevelLoaded)
{
	for (const auto Locator : LevelInstances)
	{
		if (!HasCellLevelInstance(Locator.OperationTicket, Locator.LevelInstanceSpawnGuid, bIsLevelLoaded))
		{
			return false;
		}
	}
	return true;
}

bool FNWorldAssemblyRegistry::RegisterBoneComponent(UNBoneComponent* Component)
{
	if (Bones.Contains(Component))
	{
		UE_LOG(LogNexusWorldAssembly, Verbose, TEXT("Failed to register UNBoneComponent(%s) as it is already registered; this can occur when using undo!"), *Component->GetOwner()->GetName());
		return false;
	}

	Bones.Add(Component);
	return true;
}

bool FNWorldAssemblyRegistry::RegisterCellRootComponent(UNCellRootComponent* Component)
{
	if (CellRoots.Contains(Component))
	{
		UE_LOG(LogNexusWorldAssembly, Verbose, TEXT("Failed to register UNCellRootComponent(%s) as it is already registered; this can occur when using undo!"), *Component->GetOwner()->GetName());
		return false;
	}

	CellRoots.Add(Component);
	return true;
}

bool FNWorldAssemblyRegistry::RegisterCellJunctionComponent(UNCellJunctionComponent* Component)
{
	if (CellJunctions.Contains(Component))
	{
		UE_LOG(LogNexusWorldAssembly, Verbose, TEXT("Failed to register UNCellJunctionComponent(%s) as it is already registered; this can occur when using undo!"), *Component->GetOwner()->GetName());
		return false;
	}

	CellJunctions.Add(Component);
	return true;
}

bool FNWorldAssemblyRegistry::RegisterOrganComponent(UNOrganComponent* Organ)
{
	if (Organs.Contains(Organ))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to register UNOrganComponent(%s) as it is already registered"), *Organ->GetOwner()->GetName());
		return false;
	}

	Organs.Add(Organ);
	return true;
}

bool FNWorldAssemblyRegistry::RegisterOperation(UNAssemblyOperation* Operation)
{
	if (Operations.Contains(Operation))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to register UNWorldAssemblyOperation(%s) as it is already registered"), *Operation->GetName());
		return false;
	}

	Operations.Add(Operation);
	
	NotifyOfStateChange(Operation, ENWorldAssemblyOperationState::Registered);
	return true;
}

bool FNWorldAssemblyRegistry::RegisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance)
{
	const int32 OperationTicket = CellLevelInstance->GetOperationTicket();
	const TArray<ANCellLevelInstance*>* LevelInstances = CellLevelInstances.Find(OperationTicket);
	if (LevelInstances != nullptr)
	{
		if (LevelInstances->Contains(CellLevelInstance))
		{
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to register ANCellLevelInstance(%s) as it already exists registered to OperationTicket(%i)."), *CellLevelInstance->GetName(), OperationTicket);
			return false;
		}
		CellLevelInstances[OperationTicket].Add(CellLevelInstance);
		CellLevelInstance->bRegistered = true;
		return true;
	}
	CellLevelInstances.Add(OperationTicket, TArray<ANCellLevelInstance*>());
	CellLevelInstances[OperationTicket].Add(CellLevelInstance);
	CellLevelInstance->bRegistered = true;
	return true;

}


bool FNWorldAssemblyRegistry::UnregisterBoneComponent(UNBoneComponent* Component)
{
	if (!Bones.Contains(Component))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find UNBoneComponent(%s) when attempting to unregister it."), *Component->GetOwner()->GetName());
		return false;
	}

	Bones.RemoveSwap(Component);
	return true;
}

bool FNWorldAssemblyRegistry::UnregisterCellRootComponent(UNCellRootComponent* Component)
{
	if (!CellRoots.Contains(Component))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find UNCellRootComponent(%s) when attempting to unregister it."), *Component->GetOwner()->GetName());
		return false;
	}

	CellRoots.RemoveSwap(Component);
	return true;
}

bool FNWorldAssemblyRegistry::UnregisterCellJunctionComponent(UNCellJunctionComponent* Component)
{
	if (!CellJunctions.Contains(Component))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find UNCellJunctionComponent(%s) when attempting to unregister it."), *Component->GetOwner()->GetName());
		return false;
	}

	CellJunctions.RemoveSwap(Component);
	return true;
}

bool FNWorldAssemblyRegistry::UnregisterOrganComponent(UNOrganComponent* Organ)
{
	if (!Organs.Contains(Organ))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find UNOrganComponent(%s) when attempting to unregister it."), *Organ->GetOwner()->GetName());
		return false;
	}

	Organs.RemoveSwap(Organ);
	return true;
}

bool FNWorldAssemblyRegistry::UnregisterOperation(UNAssemblyOperation* Operation)
{
	if (!Operations.Contains(Operation))
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find UNWorldAssemblyOperation(%s) when attempting to unregister it."), *Operation->GetName());
		return false;
	}

	Operations.RemoveSwap(Operation);
	NotifyOfStateChange(Operation, ENWorldAssemblyOperationState::Unregistered);
	return true;
}

bool FNWorldAssemblyRegistry::UnregisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance)
{
	const int32 OperationTicket = CellLevelInstance->GetOperationTicket();
	TArray<ANCellLevelInstance*>* LevelInstances = CellLevelInstances.Find(OperationTicket);
	if (LevelInstances != nullptr)
	{
		if (LevelInstances->Contains(CellLevelInstance))
		{
			LevelInstances->RemoveSwap(CellLevelInstance);
			CellLevelInstance->bRegistered = false;
	
			if (LevelInstances->IsEmpty())
			{
				CellLevelInstances.Remove(OperationTicket);
			}
		}
		else
		{
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find ANCellLevelInstance(%s) when attempting to unregister it from OperationTicket(%i)."), *CellLevelInstance->GetName(), OperationTicket);
			return false;
		}
	}
	else
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Failed to find the OperationTicket(%i) for ANCellLevelInstance(%s) when attempting to unregister it."), OperationTicket, *CellLevelInstance->GetName());
		return false;
	}
	return true;
}

void FNWorldAssemblyRegistry::OnPostWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// We're not going to handle Operations here, it will be handled elsewhere.
	if (IsEngineExitRequested()) return;
	
	// Scrub Bones
	for (int i = Bones.Num() - 1; i >= 0; --i)
	{
		if (Bones[i]->GetWorld() == World)
		{
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Found uncleaned bone(%s), scrubbing."), *Bones[i]->GetName());
			Bones.RemoveAt(i);
		}
	}
	
	// Scrub Cell Junctions
	for (int i = CellJunctions.Num() - 1; i >= 0; --i)
	{
		if (CellJunctions[i]->GetWorld() == World)
		{
#if WITH_EDITOR			
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Found uncleaned CellJunctions(%s), scrubbing."), *CellJunctions[i]->GetJunctionName());
#else
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Found uncleaned CellJunctions(%s), scrubbing."), *CellJunctions[i]->GetFName().ToString());
#endif // WITH_EDITOR			
			CellJunctions.RemoveAt(i);
		}
	}
	
	// Scrub Cell Roots
	for (int i = CellRoots.Num() - 1; i >= 0; --i)
	{
		if (CellRoots[i]->GetWorld() == World)
		{
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Found uncleaned CellRoot(%s), scrubbing."), *CellRoots[i]->GetName());
			CellRoots.RemoveAt(i);
		}
	}
	
	// Scrub Organs
	for (int i = Organs.Num() - 1; i >= 0; --i)
	{
		if (Organs[i]->GetWorld() == World)
		{
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Found uncleaned Organ(%s), scrubbing."), *Organs[i]->GetName());
			Organs.RemoveAt(i);
		}
	}
	
	// Scrub CellLevelInstances
	TArray<int32> OperationTickets;
	CellLevelInstances.GetKeys(OperationTickets);
	
	for (int i = OperationTickets.Num() - 1; i >= 0; --i)
	{
		const int32 Ticket = OperationTickets[i];
		const int ItemCount = CellLevelInstances[Ticket].Num();
		for (int j = ItemCount - 1; j >= 0; --j)
		{
			if (CellLevelInstances[Ticket][j]->GetWorld()== World)
			{
				UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Found uncleaned CellLevelInstances(%s), scrubbing."), *CellLevelInstances[Ticket][j]->GetName());
				CellLevelInstances[Ticket].RemoveAt(j);
			}
		}
		
		// Remove empty containers
		if (CellLevelInstances[Ticket].IsEmpty())
		{
			CellLevelInstances.Remove(Ticket);
		}
	}
}
