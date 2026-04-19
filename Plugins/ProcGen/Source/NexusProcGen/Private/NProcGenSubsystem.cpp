// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenSubsystem.h"
#include "NProcGenOperation.h"
#include "NProcGenRegistry.h"

void UNProcGenSubsystem::Generate(FNProcGenOperationSettings& Settings)
{
	UNProcGenOperation* Operation = UNProcGenOperation::CreateInstance(
		FNProcGenRegistry::GetOrganComponentsFromLevel(GetWorld()->GetCurrentLevel()), Settings);
	
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

void UNProcGenSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}
