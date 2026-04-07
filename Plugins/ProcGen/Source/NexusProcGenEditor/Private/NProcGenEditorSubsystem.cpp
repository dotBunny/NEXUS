// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorSubsystem.h"

#include "NProcGenOperation.h"
#include "Generation/NProcGenOperationSharedContext.h"

void UNProcGenEditorSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	ClearGeneratedProxies();
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);
}

void UNProcGenEditorSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenOperationSharedContext> SharedContext)
{
	
	KnownProxies.Add(Operation->GetName(), TArray<ANCellProxy*>(SharedContext->CreatedProxies));
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::ClearGeneratedProxies()
{
	if (KnownProxies.Num() > 0)
	{
		for (auto k : KnownProxies)
		{
			for (auto p : k.Value)
			{
				p->Destroy(true, false);
			}
		}
		KnownProxies.Empty();
	}
}
