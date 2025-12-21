// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NProcGenMinimal.h"
#include "NProcGenOperation.h"

class UNOrganComponent;
class UNCellJunctionComponent;
class UNCellRootComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNProcGenOperationStateChanged, UNProcGenOperation* Operation, const ENProcGenOperationState NewState);

class NEXUSPROCGEN_API FNProcGenRegistry
{
public:

	static FOnNProcGenOperationStateChanged OnOperationStateChanged;
	
	FORCEINLINE static TArray<UNCellRootComponent*>& GetCellRootComponents() { return CellRoots; }
	FORCEINLINE static TArray<UNCellJunctionComponent*>& GetCellJunctionComponents() { return CellJunctions; }
	FORCEINLINE static TArray<UNOrganComponent*>& GetOrganComponents() { return Organs; }

	static TArray<UNCellJunctionComponent*> GetCellJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted = true);
	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* Level);
	static UNCellRootComponent* GetCellRootComponentFromLevel(const ULevel* Level);
	
	static bool HasRootComponents();
	static bool HasJunctionComponents();
	static bool HasOrganComponents();
	static bool HasOrganComponentsInWorld(const UWorld* World);
	static bool HasOperations();
	
	static bool RegisterCellRootComponent(UNCellRootComponent* Component);
	static bool RegisterCellJunctionComponent(UNCellJunctionComponent* Component);
	static bool RegisterOrganComponent(UNOrganComponent* Organ);
	static bool RegisterOperation(UNProcGenOperation* Operation);
	
	static bool UnregisterCellRootComponent(UNCellRootComponent* Component);
	static bool UnregisterCellJunctionComponent(UNCellJunctionComponent* Component);
	static bool UnregisterOrganComponent(UNOrganComponent* Organ);
	static bool UnregisterOperation(UNProcGenOperation* Operation);
	
	
	
	static TArray<UNProcGenOperation*>& GetOperations() { return Operations; }
	
private:
	
	static void NotifyOfStateChange(UNProcGenOperation* Operation, ENProcGenOperationState NewState)
	{
		// We don't have a display name at registration, so the log for registration is done manually
		UE_CLOG(NewState != ENProcGenOperationState::PGOS_Registered, LogNexusProcGen, Log, TEXT("[%s] Changed State(%s)"), *Operation->GetDisplayName().ToString(), *UNProcGenOperation::GetStringFromState(NewState));
		OnOperationStateChanged.Broadcast(Operation, NewState);
	}

	
	static TArray<UNCellRootComponent*> CellRoots;
	static TArray<UNCellJunctionComponent*> CellJunctions;
	static TArray<UNOrganComponent*> Organs;
	static TArray<UNProcGenOperation*> Operations;
};