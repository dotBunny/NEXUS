// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorSubsystem.h"

#include "Assembly/NAssemblyOperation.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Organ/NOrganComponent.h"

void UNWorldAssemblyEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnMapLoadHandle = FEditorDelegates::OnMapLoad.AddUObject(this, &UNWorldAssemblyEditorSubsystem::OnMapLoad);
	PreBeginPIEHandle = FEditorDelegates::PreBeginPIE.AddUObject(this, &UNWorldAssemblyEditorSubsystem::OnPreBeginPIE);
}

void UNWorldAssemblyEditorSubsystem::Deinitialize()
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


void UNWorldAssemblyEditorSubsystem::StartOperation(UNAssemblyOperation* Operation)
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

void UNWorldAssemblyEditorSubsystem::OnOperationFinished(UNAssemblyOperation* Operation, TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext)
{
	for (ANCellProxy* Proxy : TaskGraphContext->CreatedProxies)
	{
		KnownProxies.Add(Proxy);
	}
	
	// Make our own map to the created proxies tied to the operation ticket
	ProxyMap.Add(Operation->GetTicket(), TArray<ANCellProxy*>(TaskGraphContext->CreatedProxies));
	KnownOperations.Remove(Operation);
}

void UNWorldAssemblyEditorSubsystem::OnOperationDestroyed(UNAssemblyOperation* Operation)
{
	KnownOperations.Remove(Operation);
}

void UNWorldAssemblyEditorSubsystem::ClearAllProxies()
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

void UNWorldAssemblyEditorSubsystem::ClearGenerated(const uint32& OperationTicket)
{
	ClearGeneratedProxies(OperationTicket);
}

void UNWorldAssemblyEditorSubsystem::ClearGeneratedProxies(const uint32& OperationTicket)
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

void UNWorldAssemblyEditorSubsystem::LoadAllGeneratedProxies()
{
	for (int32 i = 0; i < KnownProxies.Num(); i++)
	{
		KnownProxies[i]->LoadLevelInstance();
	}
}

void UNWorldAssemblyEditorSubsystem::LoadGeneratedProxies(const uint32& OperationTicket)
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

void UNWorldAssemblyEditorSubsystem::UnloadAllGeneratedProxies()
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

void UNWorldAssemblyEditorSubsystem::UnloadGeneratedProxies(const uint32& OperationTicket)
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

void UNWorldAssemblyEditorSubsystem::OnPreBeginPIE([[maybe_unused]] bool bArg)
{
	ClearAllProxies();
}

void UNWorldAssemblyEditorSubsystem::OnMapLoad(const FString& String, FCanLoadMap& CanLoadMap)
{
	ClearAllProxies();
}
