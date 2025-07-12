// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellRegistry.h"

#include "NCoreMinimal.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellRootComponent.h"
#include "NProcGenUtils.h"

TArray<UNCellRootComponent*> FNCellRegistry::Roots;
TArray<UNCellJunctionComponent*> FNCellRegistry::Junctions;

TArray<UNCellJunctionComponent*> FNCellRegistry::GetJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted)
{
	TArray<UNCellJunctionComponent*> JunctionComponents;
	if (Level == nullptr)
	{
		return MoveTemp(JunctionComponents);
	}

	for (UNCellJunctionComponent* Junction : GetJunctionComponents())
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



UNCellRootComponent* FNCellRegistry::GetRootComponentFromLevel(const ULevel* Level)
{
	for (UNCellRootComponent* RootComponent : GetRootComponents())
	{
		if (RootComponent->ComponentIsInLevel(Level))
		{
			return RootComponent;
		}
	}
	return nullptr;
}

bool FNCellRegistry::HasRootComponents()
{
	return Roots.Num() > 0;
}

bool FNCellRegistry::HasJunctionComponents()
{
	return Junctions.Num() > 0;
}

bool FNCellRegistry::RegisterRootComponent(UNCellRootComponent* Component)
{
	if (Roots.Contains(Component))
	{
		N_LOG(Log, TEXT("[FNCellRegistry::RegisterRootComponent] UNCellRootComponent already registered, this can occur when using undo."));
		return false;
	}

	Roots.Add(Component);
	return true;
}

bool FNCellRegistry::RegisterJunctionComponent(UNCellJunctionComponent* Component)
{
	if (Junctions.Contains(Component))
	{
		N_LOG(Log, TEXT("[FNCellRegistry::RegisterJunctionComponent] UNCellJunctionComponent already registered, this can occur when using undo."));
		return false;
	}

	Junctions.Add(Component);
	return true;
}

bool FNCellRegistry::UnregisterRootComponent(UNCellRootComponent* Component)
{
	if (!Roots.Contains(Component))
	{
		N_LOG(Warning, TEXT("[FNCellRegistry::UnregisterRootComponent] UNCellRootComponent not registered."));
		return false;
	}

	Roots.RemoveSwap(Component);
	return true;
}

bool FNCellRegistry::UnregisterJunctionComponent(UNCellJunctionComponent* Component)
{
	if (!Junctions.Contains(Component))
	{
		N_LOG(Warning, TEXT("[FNCellRegistry::UnregisterPinComponent] UNCellPinComponent not registered."));
		return false;
	}

	Junctions.RemoveSwap(Component);
	return true;
}
