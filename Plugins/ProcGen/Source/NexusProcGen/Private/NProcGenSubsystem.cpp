// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"
#include "NCoreMinimal.h"
#include "NProcGenOperation.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellProxy.h"

bool UNProcGenSubsystem::RegisterCellActor(ANCellActor* CellActor)
{
	if (KnownCellActors.Contains(CellActor))
	{
		N_LOG_WARNING("Failed to register NCellActor (%s) as it is already registered.", *CellActor->GetName());
		return false;
	}
	N_LOG_VERY_VERBOSE("Registered NCellActor (%s)", *CellActor->GetName());
	KnownCellActors.Add(CellActor);
	return true;
}

bool UNProcGenSubsystem::UnregisterCellActor(ANCellActor* CellActor)
{
	if (!KnownCellActors.Contains(CellActor))
	{
		N_LOG_WARNING("Failed to find NCellActor (%s) when attempting to unregister it.", *CellActor->GetName());
		return false;
	}
	N_LOG_VERY_VERBOSE("Unregistered NCellActor (%s).", *CellActor->GetName());
	KnownCellActors.RemoveSwap(CellActor);
	return true;
}

bool UNProcGenSubsystem::RegisterCellProxy(ANCellProxy* CellProxy)
{
	if (KnownCellProxies.Contains(CellProxy))
	{
		N_LOG_WARNING("Failed to register NCellProxy (%s) as it is already registered.", *CellProxy->GetName());
		return false;
	}
	N_LOG_VERY_VERBOSE("Registered NCellProxy (%s).", *CellProxy->GetName());
	KnownCellProxies.Add(CellProxy);
	return true;
}

bool UNProcGenSubsystem::UnregisterCellProxy(ANCellProxy* CellProxy)
{
	if (!KnownCellProxies.Contains(CellProxy))
	{
		N_LOG_WARNING("Failed to find NCellProxy (%s) when attempting to unregister it.", *CellProxy->GetName());
		return false;
	}
	N_LOG_VERY_VERBOSE("Unregistered NCellProxy (%s).", *CellProxy->GetName());
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
