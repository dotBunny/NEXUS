// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NProcGenMinimal.h"
#include "NProcGenOperation.h"
#include "Cell/NCellLevelInstanceLocator.h"

class ANCellLevelInstance;
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
	FORCEINLINE static TArray<UNBoneComponent*>& GetBoneComponents() { return Bones; }

	static TArray<UNCellJunctionComponent*> GetCellJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted = true);
	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* Level);
	static TArray<UNBoneComponent*> GetBoneComponentsFromLevel(const ULevel* Level);
	static UNCellRootComponent* GetCellRootComponentFromLevel(const ULevel* Level);

	/**
	 * Get an array of ANCellLevelInstance pointers in proximity to the provided Location.
	 * @param Location The base location to determine the distance from in world space.
	 * @param Range The maximum range to look for ANCellLevelInstances, based on the Actor transform of the ANCellLevelInstance.
	 * @param bIsLevelLoaded Is the level loaded attached to the ANCellLevelInstance?
	 * @param OperationTicket The operation ticket to query for ANCellLevelInstances, if you provide 0 in this case it will search all known ANCellLevelInstance.
	 * @return An array of ANCellLevelInstance pointers.
	 */
	static TArray<ANCellLevelInstance*> GetCellLevelInstancesInRange(const FVector& Location, double Range, bool bIsLevelLoaded = true, uint32 OperationTicket = 0);
	
	static bool HasBoneComponents();
	static bool HasRootComponents();
	static bool HasJunctionComponents();
	static bool HasOrganComponents();
	static bool HasOrganComponentsInWorld(const UWorld* World);
	static bool HasOperations();
	static bool HasCellLevelInstances(uint32 OperationTicket = 0, bool bIsLevelLoaded = true);
	static bool HasCellLevelInstances(const TArray<FNCellLevelInstanceLocator>& LevelInstances, bool bIsLevelLoaded = true);
	static bool HasCellLevelInstance(uint32 OperationTicket, FGuid LevelInstanceSpawnGuid, bool bIsLevelLoaded = true);
	
	
	static bool RegisterBoneComponent(UNBoneComponent* Component);
	static bool RegisterCellRootComponent(UNCellRootComponent* Component);
	static bool RegisterCellJunctionComponent(UNCellJunctionComponent* Component);
	static bool RegisterOrganComponent(UNOrganComponent* Organ);
	static bool RegisterOperation(UNProcGenOperation* Operation);
	static bool RegisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance);
	
	static bool UnregisterBoneComponent(UNBoneComponent* Component);
	static bool UnregisterCellRootComponent(UNCellRootComponent* Component);
	static bool UnregisterCellJunctionComponent(UNCellJunctionComponent* Component);
	static bool UnregisterOrganComponent(UNOrganComponent* Organ);
	static bool UnregisterOperation(UNProcGenOperation* Operation);
	static bool UnregisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance);
	
	static TArray<UNProcGenOperation*>& GetOperations() { return Operations; }
	
private:
	
	static void NotifyOfStateChange(UNProcGenOperation* Operation, ENProcGenOperationState NewState)
	{
		// We don't have a display name at registration, so the log for registration is done manually
		UE_CLOG(NewState != ENProcGenOperationState::Registered, LogNexusProcGen, Log, TEXT("[%s] Changed State(%s)"), *Operation->GetDisplayName().ToString(), *UNProcGenOperation::GetStringFromState(NewState));
		OnOperationStateChanged.Broadcast(Operation, NewState);
	}
	
	static TArray<UNBoneComponent*> Bones;
	static TArray<UNCellRootComponent*> CellRoots;
	static TArray<UNCellJunctionComponent*> CellJunctions;
	static TArray<UNOrganComponent*> Organs;
	static TArray<UNProcGenOperation*> Operations;
	static TMap<uint32, TArray<ANCellLevelInstance*>> CellLevelInstances;
};