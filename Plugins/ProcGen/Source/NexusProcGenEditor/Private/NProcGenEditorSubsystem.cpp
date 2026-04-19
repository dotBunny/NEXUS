// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorSubsystem.h"

#include "NProcGenOperation.h"
#include "Generation/NProcGenTaskGraphContext.h"
#include "Organ/NOrganComponent.h"

void UNProcGenEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnMapLoadHandle = FEditorDelegates::OnMapLoad.AddUObject(this, &UNProcGenEditorSubsystem::OnMapLoad);
	PreBeginPIEHandle = FEditorDelegates::PreBeginPIE.AddUObject(this, &UNProcGenEditorSubsystem::OnPreBeginPIE);
}

void UNProcGenEditorSubsystem::Deinitialize()
{
	if (HasGeneratedCellProxies())
	{
		ClearAllProxies();
	}

	FEditorDelegates::OnMapLoad.Remove(OnMapLoadHandle);
	OnMapLoadHandle.Reset();
	
	FEditorDelegates::PreBeginPIE.Remove(PreBeginPIEHandle);
	PreBeginPIEHandle.Reset();
	
	Super::Deinitialize();
}


void UNProcGenEditorSubsystem::StartOperation(UNProcGenOperation* Operation)
{
	// Clear for anything in operation
	for (UNOrganComponent* Component :  Operation->Context->InputComponents)
	{
		FGuid Guid = Component->GetAndResetOperationIdentifier();
		if (Guid.IsValid())
		{
			ClearGenerated(Guid);
		}
	}
	
	KnownOperations.AddUnique(Operation);
	Operation->StartBuild(this);
}

void UNProcGenEditorSubsystem::OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext)
{
	for (ANCellProxy* Proxy : TaskGraphContext->CreatedProxies)
	{
		KnownProxies.Add(Proxy);
	}
	ProxyMap.Add(Operation->GetGuid(), TArray<ANCellProxy*>(TaskGraphContext->CreatedProxies)); // TODO : Check this copies?
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::ClearAllProxies()
{
	for (int i = 0; i < KnownProxies.Num(); i++)
	{
		if (IsValid(KnownProxies[i]))
		{
			KnownProxies[i]->DestroyLevelInstance();
			KnownProxies[i]->Destroy(true, false);
		}
	}
	ProxyMap.Empty();
	KnownProxies.Empty();
}

void UNProcGenEditorSubsystem::ClearGenerated(const FGuid& Guid)
{
	ClearGeneratedProxies(Guid);
}

void UNProcGenEditorSubsystem::ClearGeneratedProxies(const FGuid& Guid)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(Guid))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(Guid);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			KnownProxies.Remove(ProxiesArray[i]);
			if (IsValid(ProxiesArray[i]))
			{
				ProxiesArray[i]->DestroyLevelInstance();
				ProxiesArray[i]->Destroy(true, false);
			}
		}
		ProxyMap.Remove(Guid);
	}
}

void UNProcGenEditorSubsystem::LoadAllGeneratedProxies()
{
	for (int i = 0; i < KnownProxies.Num(); i++)
	{
		KnownProxies[i]->LoadLevelInstance();
	}
}

void UNProcGenEditorSubsystem::LoadGeneratedProxies(const FGuid& Guid)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(Guid))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(Guid);
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
		TArray<FGuid> KnownKeys;
		ProxyMap.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			UnloadGeneratedProxies(Key);
		}
	}
}

void UNProcGenEditorSubsystem::UnloadGeneratedProxies(const FGuid& Guid)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(Guid))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(Guid);
		const int FoundCount = ProxiesArray.Num();
		for (int i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->UnloadLevelInstance();
		}
	}
}

void UNProcGenEditorSubsystem::OnPreBeginPIE(bool bArg)
{
	ClearAllProxies();
}

void UNProcGenEditorSubsystem::OnMapLoad(const FString& String, FCanLoadMap& CanLoadMap)
{
	ClearAllProxies();
}
