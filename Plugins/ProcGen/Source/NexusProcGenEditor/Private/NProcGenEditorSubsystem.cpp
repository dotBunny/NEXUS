// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorSubsystem.h"

#include "NProcGenOperation.h"
#include "Generation/Contexts/NProcGenTaskGraphContext.h"
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
		uint32 LastOperationTicket = Component->GetAndResetLastOperationTicket();
		if (LastOperationTicket != 0)
		{
			ClearGenerated(LastOperationTicket);
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
	
	// Make our own map to the created proxies tied to the operation ticket
	ProxyMap.Add(Operation->GetTicket(), TArray<ANCellProxy*>(TaskGraphContext->CreatedProxies));
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::OnOperationDestroyed(UNProcGenOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNProcGenEditorSubsystem::ClearAllProxies()
{
	for (int32 i = 0; i < KnownProxies.Num(); i++)
	{
		if (IsValid(KnownProxies[i]))
		{
			KnownProxies[i]->DestroyLevelInstance(true);
			KnownProxies[i]->Destroy(true, false);
		}
	}
	ProxyMap.Empty();
	KnownProxies.Empty();
}

void UNProcGenEditorSubsystem::ClearGenerated(const uint32& OperationTicket)
{
	ClearGeneratedProxies(OperationTicket);
}

void UNProcGenEditorSubsystem::ClearGeneratedProxies(const uint32& OperationTicket)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(OperationTicket))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(OperationTicket);
		const int32 FoundCount = ProxiesArray.Num();
		for (int32 i = 0; i < FoundCount; i++)
		{
			KnownProxies.Remove(ProxiesArray[i]);
			if (IsValid(ProxiesArray[i]))
			{
				ProxiesArray[i]->DestroyLevelInstance(true);
				ProxiesArray[i]->Destroy(true, false);
			}
		}
		ProxyMap.Remove(OperationTicket);
	}
}

void UNProcGenEditorSubsystem::LoadAllGeneratedProxies()
{
	for (int32 i = 0; i < KnownProxies.Num(); i++)
	{
		KnownProxies[i]->LoadLevelInstance();
	}
}

void UNProcGenEditorSubsystem::LoadGeneratedProxies(const uint32& OperationTicket)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(OperationTicket))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(OperationTicket);
		const int32 FoundCount = ProxiesArray.Num();
		for (int32 i = 0; i < FoundCount; i++)
		{
			ProxiesArray[i]->LoadLevelInstance();
		}
	}
}

void UNProcGenEditorSubsystem::UnloadAllGeneratedProxies()
{
	if (ProxyMap.Num() > 0)
	{
		TArray<uint32> KnownKeys;
		ProxyMap.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			UnloadGeneratedProxies(Key);
		}
	}
}

void UNProcGenEditorSubsystem::UnloadGeneratedProxies(const uint32& OperationTicket)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(OperationTicket))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(OperationTicket);
		const int32 FoundCount = ProxiesArray.Num();
		for (int32 i = 0; i < FoundCount; i++)
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
