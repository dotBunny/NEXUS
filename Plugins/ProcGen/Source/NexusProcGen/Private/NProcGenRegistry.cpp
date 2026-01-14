// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenRegistry.h"

#include "NProcGenMinimal.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellRootComponent.h"
#include "Organ/NOrganComponent.h"

TArray<UNCellRootComponent*> FNProcGenRegistry::CellRoots;
TArray<UNCellJunctionComponent*> FNProcGenRegistry::CellJunctions;
TArray<UNOrganComponent*> FNProcGenRegistry::Organs;

TArray<UNProcGenOperation*> FNProcGenRegistry::Operations;
FOnNProcGenOperationStateChanged FNProcGenRegistry::OnOperationStateChanged;

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
