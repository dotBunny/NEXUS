// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"
#include "NCoreMinimal.h"

bool UNProcGenSubsystem::RegisterNCellActor(ANCellActor* NCellActor)
{
	if (KnownNCellActors.Contains(NCellActor))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::RegisterNCellActor] NCellActor already registered."));
		return false;
	}

	N_LOG(Log, TEXT("[UNProcGenSubsystem::RegisterNCellActor] NCellActor registered."));
	KnownNCellActors.Add(NCellActor);
	return true;
}

bool UNProcGenSubsystem::UnregisterNCellActor(ANCellActor* NCellActor)
{
	if (!KnownNCellActors.Contains(NCellActor))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::UnregisterNCellActor] NCellActor not registered."));
		return false;
	}

	N_LOG(Log, TEXT("[UNProcGenSubsystem::UnregisterNCellActor] NCellActor unregistered."));
	KnownNCellActors.RemoveSwap(NCellActor);
	return true;
}

bool UNProcGenSubsystem::RegisterNCellProxy(ANCellProxy* NCellProxy)
{
	if (KnownNCellProxies.Contains(NCellProxy))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::RegisterNCellProxy] NCellProxy already registered."));
		return false;
	}

	N_LOG(Log, TEXT("[UNProcGenSubsystem::RegisterNCellActor] NCellProxy registered."));
	KnownNCellProxies.Add(NCellProxy);
	return true;
}

bool UNProcGenSubsystem::UnregisterNCellProxy(ANCellProxy* NCellProxy)
{
	if (!KnownNCellProxies.Contains(NCellProxy))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::UnregisterNCellProxy] NCellProxy not registered."));
		return false;
	}
	N_LOG(Log, TEXT("[UNProcGenSubsystem::UnregisterNCellProxy] NCellProxy unregistered."));
	KnownNCellProxies.RemoveSwap(NCellProxy);
	return true;
}
