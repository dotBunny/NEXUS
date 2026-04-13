// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"
#include "NProcGenOperation.h"
#include "NProcGenRegistry.h"

void UNProcGenSubsystem::Generate(FString Seed, FText OperationName, const bool bLoadAll)
{
	UNProcGenOperation* Operation = UNProcGenOperation::CreateInstance(
		FNProcGenRegistry::GetOrganComponentsFromLevel(GetWorld()->GetCurrentLevel()), Seed, OperationName); 
	
	if (bLoadAll)
	{
		Operation->SetCreateLevelInstances(true);
		Operation->SetLoadLevelInstances(true);
	}
	
	StartOperation(Operation);
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

void UNProcGenSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenOperationSharedContext> SharedContext)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}
