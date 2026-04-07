// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorSubsystem.h"

#include "NProcGenOperation.h"
#include "Generation/NProcGenOperationSharedContext.h"
#include "Organ/NOrganComponent.h"

void UNProcGenEditorSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	// Clear for anything in operation
	for (const UNOrganComponent* Component :  Operation->Context->InputComponents)
	{
		FName LastGenerationKey = Component->GetLastGenerationOperationKey();
		if (LastGenerationKey != NAME_None)
		{
			ClearGeneratedProxies(LastGenerationKey);
		}
	}
	
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);
}

void UNProcGenEditorSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenOperationSharedContext> SharedContext)
{
	KnownProxies.Add(Operation->GetFName(), TArray<ANCellProxy*>(SharedContext->CreatedProxies)); // TODO : Check this copies?
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::ClearAllGeneratedProxies()
{
	if (KnownProxies.Num() > 0)
	{
		for (auto KVP : KnownProxies)
		{
			for (const auto Proxy : KVP.Value)
			{
				Proxy->Destroy(true, false);
			}
		}
		KnownProxies.Empty();
	}
}

void UNProcGenEditorSubsystem::ClearGeneratedProxies(const FName& Key)
{
	if (KnownProxies.Num() > 0 && KnownProxies.Contains(Key))
	{
		TArray<ANCellProxy*> ProxiesArray = *KnownProxies.Find(Key);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->Destroy(true, false);
		}
		KnownProxies.Remove(Key);
	}
}
