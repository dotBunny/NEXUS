// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"
#include "NCoreMinimal.h"
#include "NProcGenOperation.h"
#include "NProcGenOperation.h"

bool UNProcGenSubsystem::RegisterCellActor(ANCellActor* CellActor)
{
	if (KnownCellActors.Contains(CellActor))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::RegisterNCellActor] NCellActor already registered."));
		return false;
	}

	N_LOG(Log, TEXT("[UNProcGenSubsystem::RegisterNCellActor] NCellActor registered."));
	KnownCellActors.Add(CellActor);
	return true;
}

bool UNProcGenSubsystem::UnregisterCellActor(ANCellActor* CellActor)
{
	if (!KnownCellActors.Contains(CellActor))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::UnregisterNCellActor] NCellActor not registered."));
		return false;
	}

	N_LOG(Log, TEXT("[UNProcGenSubsystem::UnregisterNCellActor] NCellActor unregistered."));
	KnownCellActors.RemoveSwap(CellActor);
	return true;
}

bool UNProcGenSubsystem::RegisterCellProxy(ANCellProxy* CellProxy)
{
	if (KnownCellProxies.Contains(CellProxy))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::RegisterNCellProxy] NCellProxy already registered."));
		return false;
	}

	N_LOG(Log, TEXT("[UNProcGenSubsystem::RegisterNCellActor] NCellProxy registered."));
	KnownCellProxies.Add(CellProxy);
	return true;
}

bool UNProcGenSubsystem::UnregisterCellProxy(ANCellProxy* CellProxy)
{
	if (!KnownCellProxies.Contains(CellProxy))
	{
		N_LOG(Warning, TEXT("[UNProcGenSubsystem::UnregisterNCellProxy] NCellProxy not registered."));
		return false;
	}
	N_LOG(Log, TEXT("[UNProcGenSubsystem::UnregisterNCellProxy] NCellProxy unregistered."));
	KnownCellProxies.RemoveSwap(CellProxy);
	return true;
}

void UNProcGenSubsystem::Tick(float DeltaTime)
{
	
}

bool UNProcGenSubsystem::IsTickable() const
{
	if (KnownOperations.Num() > 0) return true;
	return false;
}

void UNProcGenSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);
}

void UNProcGenSubsystem::OnOperationFinished(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}
