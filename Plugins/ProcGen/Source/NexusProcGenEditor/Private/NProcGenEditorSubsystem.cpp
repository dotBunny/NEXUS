#include "NProcGenEditorSubsystem.h"

#include "NProcGenOperation.h"

void UNProcGenEditorSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);
}

void UNProcGenEditorSubsystem::OnOperationFinished(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}