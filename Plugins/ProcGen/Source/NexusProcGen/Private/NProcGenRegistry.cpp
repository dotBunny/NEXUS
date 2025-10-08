// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenRegistry.h"

#include "NCoreMinimal.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellRootComponent.h"
#include "Organ/NOrganComponent.h"

TArray<UNCellRootComponent*> FNProcGenRegistry::CellRoots;
TArray<UNCellJunctionComponent*> FNProcGenRegistry::CellJunctions;
TArray<UNOrganComponent*> FNProcGenRegistry::Organs;

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

bool FNProcGenRegistry::RegisterCellRootComponent(UNCellRootComponent* Component)
{
	if (CellRoots.Contains(Component))
	{
		N_LOG(Log, TEXT("[FNCellRegistry::RegisterRootComponent] UNCellRootComponent already registered, this can occur when using undo."));
		return false;
	}

	CellRoots.Add(Component);
	return true;
}

bool FNProcGenRegistry::RegisterCellJunctionComponent(UNCellJunctionComponent* Component)
{
	if (CellJunctions.Contains(Component))
	{
		N_LOG(Log, TEXT("[FNCellRegistry::RegisterJunctionComponent] UNCellJunctionComponent already registered, this can occur when using undo."));
		return false;
	}

	CellJunctions.Add(Component);
	return true;
}

bool FNProcGenRegistry::RegisterOrganComponent(UNOrganComponent* Organ)
{
	if (Organs.Contains(Organ))
	{
		N_LOG(Warning, TEXT("[FNCellRegistry::RegisterOrganComponent] UNOrganComponent already registered."));
		return false;
	}

	Organs.Add(Organ);
	return true;
}

bool FNProcGenRegistry::UnregisterCellRootComponent(UNCellRootComponent* Component)
{
	if (!CellRoots.Contains(Component))
	{
		N_LOG(Warning, TEXT("[FNCellRegistry::UnregisterRootComponent] UNCellRootComponent not registered."));
		return false;
	}

	CellRoots.RemoveSwap(Component);
	return true;
}

bool FNProcGenRegistry::UnregisterCellJunctionComponent(UNCellJunctionComponent* Component)
{
	if (!CellJunctions.Contains(Component))
	{
		N_LOG(Warning, TEXT("[FNCellRegistry::UnregisterPinComponent] UNCellPinComponent not registered."));
		return false;
	}

	CellJunctions.RemoveSwap(Component);
	return true;
}

bool FNProcGenRegistry::UnregisterOrganComponent(UNOrganComponent* Organ)
{
	if (!Organs.Contains(Organ))
	{
		N_LOG(Warning, TEXT("[FNCellRegistry::UnregisterOrganComponent] UNOrganComponent not registered."));
		return false;
	}

	Organs.RemoveSwap(Organ);
	return true;
}
