// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorSubsystem.h"

#include "NProcGenOperation.h"
#include "Generation/NProcGenOperationSharedContext.h"
#include "Organ/NOrganComponent.h"

void UNProcGenEditorSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	// Clear for anything in operation
	for (UNOrganComponent* Component :  Operation->Context->InputComponents)
	{
		FName LastGenerationKey = Component->GetAndResetGenerationOperationKey();
		if (LastGenerationKey != NAME_None)
		{
			ClearGenerated(LastGenerationKey);
		}
	}
	
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);
}

void UNProcGenEditorSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenOperationSharedContext> SharedContext)
{
	for (ANCellProxy* Proxy : SharedContext->CreatedProxies)
	{
		KnownProxies.Add(Proxy);
	}
	ProxyMap.Add(Operation->GetFName(), TArray<ANCellProxy*>(SharedContext->CreatedProxies)); // TODO : Check this copies?
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::ClearAllProxies()
{
	if (ProxyMap.Num() > 0)
	{
		TArray<FName> KnownKeys;
		ProxyMap.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			ClearGeneratedProxies(Key);
		}
		ProxyMap.Empty();
		KnownProxies.Empty();
	}
}

void UNProcGenEditorSubsystem::ClearGeneratedProxies(const FName& Key)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(Key))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(Key);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			KnownProxies.Remove(ProxiesArray[i]);
			ProxiesArray[i]->DestroyLevelInstance();
			ProxiesArray[i]->Destroy(true, false);
		}
		ProxyMap.Remove(Key);
	}
}

void UNProcGenEditorSubsystem::LoadAllGeneratedProxies()
{
	if (ProxyMap.Num() > 0)
	{
		TArray<FName> KnownKeys;
		ProxyMap.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			LoadGeneratedProxies(Key);
		}
	}
}

void UNProcGenEditorSubsystem::LoadGeneratedProxies(const FName& Key)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(Key))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(Key);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->LoadLevelInstance();
		}
	}
}

void UNProcGenEditorSubsystem::UnloadAllGeneratedProxies()
{
	if (ProxyMap.Num() > 0)
	{
		TArray<FName> KnownKeys;
		ProxyMap.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			UnloadGeneratedProxies(Key);
		}
	}
}

void UNProcGenEditorSubsystem::UnloadGeneratedProxies(const FName& Key)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(Key))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(Key);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->UnloadLevelInstance();
		}
	}
}
