// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"
#include "NProcGenMinimal.h"
#include "NProcGenOperation.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellProxy.h"

bool UNProcGenSubsystem::RegisterCellActor(ANCellActor* CellActor)
{
	if (KnownCellActors.Contains(CellActor))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to register ANCellActor(%s) as it is already registered."), *CellActor->GetName());
		return false;
	}
	UE_LOG(LogNexusProcGen, VeryVerbose, TEXT("Registered ANCellActor(%s)."), *CellActor->GetName());
	KnownCellActors.Add(CellActor);
	return true;
}

bool UNProcGenSubsystem::UnregisterCellActor(ANCellActor* CellActor)
{
	if (!KnownCellActors.Contains(CellActor))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find ANCellActor(%s) when attempting to unregister it."), *CellActor->GetName());
		return false;
	}
	UE_LOG(LogNexusProcGen, VeryVerbose, TEXT("Unregistered ANCellActor(%s)."), *CellActor->GetName());
	KnownCellActors.RemoveSwap(CellActor);
	return true;
}

bool UNProcGenSubsystem::RegisterCellProxy(ANCellProxy* CellProxy)
{
	if (KnownCellProxies.Contains(CellProxy))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to register ANCellProxy(%s) as it is already registered."), *CellProxy->GetName());
		return false;
	}
	UE_LOG(LogNexusProcGen, VeryVerbose, TEXT("Registered ANCellProxy(%s)."), *CellProxy->GetName());
	KnownCellProxies.Add(CellProxy);
	return true;
}

bool UNProcGenSubsystem::UnregisterCellProxy(ANCellProxy* CellProxy)
{
	if (!KnownCellProxies.Contains(CellProxy))
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Failed to find ANCellProxy(%s) when attempting to unregister it."), *CellProxy->GetName());
		return false;
	}
	UE_LOG(LogNexusProcGen, VeryVerbose, TEXT("Unregistered ANCellProxy(%s)."), *CellProxy->GetName());
	KnownCellProxies.RemoveSwap(CellProxy);
	return true;
}

void UNProcGenSubsystem::Tick(float DeltaTime)
{
	for (const auto Operation : KnownOperations)
	{
		Operation->Tick();
	}
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

void UNProcGenSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}
