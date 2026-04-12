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
	KnownProxies.Add(Operation->GetFName(), TArray<ANCellProxy*>(SharedContext->CreatedProxies)); // TODO : Check this copies?
	KnownActors.Add(Operation->GetFName(), TArray<AActor*>(SharedContext->CreatedActors)); // TODO : Check this copies?
	
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::ClearAllGenerated()
{
	if (KnownProxies.Num() > 0)
	{
		TArray<FName> KnownKeys;
		KnownProxies.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			ClearGeneratedProxies(Key);
		}
		KnownProxies.Empty();
	}
	
	
	if (KnownActors.Num() > 0)
	{
		TArray<FName> KnownKeys;
		KnownActors.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			ClearGeneratedActors(Key);
		}
		KnownActors.Empty();
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
			ProxiesArray[i]->DestroyLevelInstance();
			ProxiesArray[i]->Destroy(true, false);
		}
		KnownProxies.Remove(Key);
	}
}

void UNProcGenEditorSubsystem::ClearGeneratedActors(const FName& Key)
{
	if (KnownActors.Num() > 0 && KnownActors.Contains(Key))
	{
		TArray<AActor*> ActorsArray = *KnownActors.Find(Key);
		const int FoundCount = ActorsArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ActorsArray[i]->Destroy(true, false);
		}
		KnownActors.Remove(Key);
	}
}

void UNProcGenEditorSubsystem::LoadAllGeneratedProxies()
{
	if (KnownProxies.Num() > 0)
	{
		TArray<FName> KnownKeys;
		KnownProxies.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			LoadGeneratedProxies(Key);
		}
	}
}

void UNProcGenEditorSubsystem::LoadGeneratedProxies(const FName& Key)
{
	if (KnownProxies.Num() > 0 && KnownProxies.Contains(Key))
	{
		TArray<ANCellProxy*> ProxiesArray = *KnownProxies.Find(Key);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->LoadLevelInstance();
		}
	}
}

void UNProcGenEditorSubsystem::UnloadAllGeneratedProxies()
{
	if (KnownProxies.Num() > 0)
	{
		TArray<FName> KnownKeys;
		KnownProxies.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			UnloadGeneratedProxies(Key);
		}
	}
}

void UNProcGenEditorSubsystem::UnloadGeneratedProxies(const FName& Key)
{
	if (KnownProxies.Num() > 0 && KnownProxies.Contains(Key))
	{
		TArray<ANCellProxy*> ProxiesArray = *KnownProxies.Find(Key);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->UnloadLevelInstance();
		}
	}
}
