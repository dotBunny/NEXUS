// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenRegistry.h"

#include "NProcGenMinimal.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellLevelInstance.h"
#include "Cell/NCellRootComponent.h"
#include "Organ/NOrganComponent.h"

TArray<UNCellRootComponent*> FNProcGenRegistry::CellRoots;
TArray<UNBoneComponent*> FNProcGenRegistry::Bones;
TArray<UNCellJunctionComponent*> FNProcGenRegistry::CellJunctions;
TArray<UNOrganComponent*> FNProcGenRegistry::Organs;

TArray<UNProcGenOperation*> FNProcGenRegistry::Operations;
FOnNProcGenOperationStateChanged FNProcGenRegistry::OnOperationStateChanged;
TMap<uint32, TArray<ANCellLevelInstance*>> FNProcGenRegistry::CellLevelInstances;

TArray<UNCellJunctionComponent*> FNProcGenRegistry::GetCellJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted)
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

TArray<UNOrganComponent*> FNProcGenRegistry::GetOrganComponentsFromLevel(const ULevel* Level)
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

	return MoveTemp(OrganComponents);
}

TArray<UNBoneComponent*> FNProcGenRegistry::GetBoneComponentsFromLevel(const ULevel* Level)
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

	return MoveTemp(BoneComponents);
}

UNCellRootComponent* FNProcGenRegistry::GetCellRootComponentFromLevel(const ULevel* Level)
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

bool FNProcGenRegistry::HasBoneComponents()
{
	return Bones.Num() > 0;
}

bool FNProcGenRegistry::HasRootComponents()
{
	return CellRoots.Num() > 0;
}

bool FNProcGenRegistry::HasJunctionComponents()
{
	return CellJunctions.Num() > 0;
}

bool FNProcGenRegistry::HasOrganComponents()
{
	return Organs.Num() > 0;
}

bool FNProcGenRegistry::HasOrganComponentsInWorld(const UWorld* World)
{
	for (const UNOrganComponent* Component : GetOrganComponents())
	{
		if (Component->GetWorld() == World) return true;
	}
	return false;
}

bool FNProcGenRegistry::HasOperations()
{
	return Operations.Num() > 0;
}

bool FNProcGenRegistry::HasCellLevelInstances(const uint32 OperationTicket, const bool bIsLevelLoaded)
{
	if (OperationTicket == 0)
	{
		return CellLevelInstances.Num() > 0;
	}
	
	if (CellLevelInstances.Contains(OperationTicket))
	{
		return CellLevelInstances[OperationTicket].Num() > 0;
	}
	return false;
}

TArray<ANCellLevelInstance*> FNProcGenRegistry::GetCellLevelInstancesInRange(const FVector& Location, const double Range, const bool bIsLevelLoaded, const uint32 OperationTicket)
{
	TArray<ANCellLevelInstance*> Results;
	if (CellLevelInstances.Num() == 0) return MoveTemp(Results);
	
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
		if (const TArray<ANCellLevelInstance*>* Ptr = CellLevelInstances.Find(OperationTicket))
		{
			AppendInRange(*Ptr);
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

bool FNProcGenRegistry::HasCellLevelInstance(const uint32 OperationTicket, const FGuid LevelInstanceSpawnGuid, const bool bIsLevelLoaded)
{
	if (TArray<ANCellLevelInstance*>* LevelInstances = CellLevelInstances.Find(OperationTicket))
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

bool FNProcGenRegistry::HasCellLevelInstances(const TArray<FNCellLevelInstanceLocator>& LevelInstances, const bool bIsLevelLoaded)
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

bool FNProcGenRegistry::RegisterBoneComponent(UNBoneComponent* Component)
{
	if (Bones.Contains(Component))
	{
		UE_LOG(LogNexusProcGen, Verbose, TEXT("Failed to register UNBoneComponent(%s) as it is already registered; this can occur when using undo!"), *Component->GetOwner()->GetName());
		return false;
	}

	Bones.Add(Component);
	return true;
}

bool FNProcGenRegistry::RegisterCellRootComponent(UNCellRootComponent* Component)
{
	if (CellRoots.Contains(Component))
	{
		UE_LOG(LogNexusProcGen, Verbose, TEXT("Failed to register UNCellRootComponent(%s) as it is already registered; this can occur when using undo!"), *Component->GetOwner()->GetName());
		return false;
	}

	CellRoots.Add(Component);
	return true;
}

bool FNProcGenRegistry::RegisterCellJunctionComponent(UNCellJunctionComponent* Component)
{
	if (CellJunctions.Contains(Component))
	{
		UE_LOG(LogNexusProcGen, Verbose, TEXT("Failed to register UNCellJunctionComponent(%s) as it is already registered; this can occur when using undo!"), *Component->GetOwner()->GetName());
		return false;
	}

	CellJunctions.Add(Component);
	return true;
}

bool FNProcGenRegistry::RegisterOrganComponent(UNOrganComponent* Organ)
{
	if (Organs.Contains(Organ))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to register UNOrganComponent(%s) as it is already registered"), *Organ->GetOwner()->GetName());
		return false;
	}

	Organs.Add(Organ);
	return true;
}

bool FNProcGenRegistry::RegisterOperation(UNProcGenOperation* Operation)
{
	if (Operations.Contains(Operation))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to register UNProcGenOperation(%s) as it is already registered"), *Operation->GetName());
		return false;
	}

	Operations.Add(Operation);
	
	NotifyOfStateChange(Operation, ENProcGenOperationState::Registered);
	return true;
}

bool FNProcGenRegistry::RegisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance)
{
	const uint32 OperationTicket = CellLevelInstance->GetOperationTicket();
	if (const TArray<ANCellLevelInstance*>* LevelInstances = CellLevelInstances.Find(OperationTicket))
	{
		if (LevelInstances->Contains(CellLevelInstance))
		{
			UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to register ANCellLevelInstance(%s) as it already exists registered to OperationTicket(%i)."), *CellLevelInstance->GetName(), OperationTicket);
			return false;
		}
		CellLevelInstances[OperationTicket].Add(CellLevelInstance);
		return true;
	}
	CellLevelInstances.Add(OperationTicket, TArray<ANCellLevelInstance*>());
	CellLevelInstances[OperationTicket].Add(CellLevelInstance);
	return true;

}

bool FNProcGenRegistry::UnregisterBoneComponent(UNBoneComponent* Component)
{
	if (!Bones.Contains(Component))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find UNBoneComponent(%s) when attempting to unregister it."), *Component->GetOwner()->GetName());
		return false;
	}

	Bones.RemoveSwap(Component);
	return true;
}

bool FNProcGenRegistry::UnregisterCellRootComponent(UNCellRootComponent* Component)
{
	if (!CellRoots.Contains(Component))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find UNCellRootComponent(%s) when attempting to unregister it."), *Component->GetOwner()->GetName());
		return false;
	}

	CellRoots.RemoveSwap(Component);
	return true;
}

bool FNProcGenRegistry::UnregisterCellJunctionComponent(UNCellJunctionComponent* Component)
{
	if (!CellJunctions.Contains(Component))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find UNCellJunctionComponent(%s) when attempting to unregister it."), *Component->GetOwner()->GetName());
		return false;
	}

	CellJunctions.RemoveSwap(Component);
	return true;
}

bool FNProcGenRegistry::UnregisterOrganComponent(UNOrganComponent* Organ)
{
	if (!Organs.Contains(Organ))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find UNOrganComponent(%s) when attempting to unregister it."), *Organ->GetOwner()->GetName());
		return false;
	}

	Organs.RemoveSwap(Organ);
	return true;
}

bool FNProcGenRegistry::UnregisterOperation(UNProcGenOperation* Operation)
{
	if (!Operations.Contains(Operation))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find UNProcGenOperation(%s) when attempting to unregister it."), *Operation->GetName());
		return false;
	}

	Operations.RemoveSwap(Operation);
	NotifyOfStateChange(Operation, ENProcGenOperationState::Unregistered);
	return true;
}

bool FNProcGenRegistry::UnregisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance)
{
	const uint32 OperationTicket = CellLevelInstance->GetOperationTicket();
	if (TArray<ANCellLevelInstance*>* LevelInstances = CellLevelInstances.Find(OperationTicket))
	{
		if (LevelInstances->Contains(CellLevelInstance))
		{
			LevelInstances->RemoveSwap(CellLevelInstance);
			if (LevelInstances->Num() == 0)
			{
				CellLevelInstances.Remove(OperationTicket);
			}
		}
		else
		{
			UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find ANCellLevelInstance(%s) when attempting to unregister it from OperationTicket(%i)."), *CellLevelInstance->GetName(), OperationTicket);
			return false;
		}
	}
	else
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find the OperationTicket(%i) for ANCellLevelInstance(%s) when attempting to unregister it."), OperationTicket, *CellLevelInstance->GetName());
		return false;
	}
	return true;
}
