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

/** Broadcast whenever a ProcGen operation's lifecycle state changes. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNProcGenOperationStateChanged, UNProcGenOperation* Operation, const ENProcGenOperationState NewState);


/**
 * Process-wide registry of the components, operations and level instances that participate in ProcGen.
 *
 * Components self-register via the Register/Unregister pair during their lifecycle, which lets
 * subsystems and tooling enumerate the active ProcGen graph without walking every actor/component.
 * @note All accessors expect to be called from the game thread.
 */
class NEXUSPROCGEN_API FNProcGenRegistry
{
public:
	/** Broadcast when any operation transitions between lifecycle states. */
	static FOnNProcGenOperationStateChanged OnOperationStateChanged;

	/** @return All registered cell root components. */
	FORCEINLINE static TArray<UNCellRootComponent*>& GetCellRootComponents() { return CellRoots; }
	/** @return All registered cell junction components. */
	FORCEINLINE static TArray<UNCellJunctionComponent*>& GetCellJunctionComponents() { return CellJunctions; }
	/** @return All registered organ components. */
	FORCEINLINE static TArray<UNOrganComponent*>& GetOrganComponents() { return Organs; }
	/** @return All registered bone components. */
	FORCEINLINE static TArray<UNBoneComponent*>& GetBoneComponents() { return Bones; }

	/**
	 * @param Level Level to query.
	 * @param bSorted When true, results are returned in a stable sort order suitable for deterministic traversal.
	 * @return Cell junctions whose owning actor belongs to the supplied level.
	 */
	static TArray<UNCellJunctionComponent*> GetCellJunctionsComponentsFromLevel(const ULevel* Level, const bool bSorted = true);
	/** @return Organ components whose owning actor belongs to the supplied level. */
	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* Level);
	/** @return Bone components whose owning actor belongs to the supplied level. */
	static TArray<UNBoneComponent*> GetBoneComponentsFromLevel(const ULevel* Level);
	/** @return The single cell root component belonging to the supplied level, or nullptr if none is registered. */
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
	
	/** @return true if any bone components are currently registered. */
	static bool HasBoneComponents();
	/** @return true if any cell root components are currently registered. */
	static bool HasRootComponents();
	/** @return true if any cell junction components are currently registered. */
	static bool HasJunctionComponents();
	/** @return true if any organ components are currently registered. */
	static bool HasOrganComponents();
	/** @return true if at least one registered organ component belongs to an actor in the supplied world. */
	static bool HasOrganComponentsInWorld(const UWorld* World);
	/** @return true if any ProcGen operations are currently known. */
	static bool HasOperations();
	/**
	 * @param OperationTicket Ticket to scope the query; 0 queries across all operations.
	 * @param bIsLevelLoaded Consider only level instances whose underlying level has finished loading.
	 */
	static bool HasCellLevelInstances(uint32 OperationTicket = 0, bool bIsLevelLoaded = true);
	/** Query whether every locator in the supplied set is currently registered. */
	static bool HasCellLevelInstances(const TArray<FNCellLevelInstanceLocator>& LevelInstances, bool bIsLevelLoaded = true);
	/** Query whether a specific (operation, spawn GUID) level instance is registered. */
	static bool HasCellLevelInstance(uint32 OperationTicket, FGuid LevelInstanceSpawnGuid, bool bIsLevelLoaded = true);


	/** Register a bone component with the registry. @return true on first registration. */
	static bool RegisterBoneComponent(UNBoneComponent* Component);
	/** Register a cell root component with the registry. @return true on first registration. */
	static bool RegisterCellRootComponent(UNCellRootComponent* Component);
	/** Register a cell junction component with the registry. @return true on first registration. */
	static bool RegisterCellJunctionComponent(UNCellJunctionComponent* Component);
	/** Register an organ component with the registry. @return true on first registration. */
	static bool RegisterOrganComponent(UNOrganComponent* Organ);
	/** Register an operation and broadcast OnOperationStateChanged with Registered. @return true on first registration. */
	static bool RegisterOperation(UNProcGenOperation* Operation);
	/** Register a cell level instance with the registry, indexed by its operation ticket. @return true on first registration. */
	static bool RegisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance);

	/** Unregister a bone component. @return true if it was removed. */
	static bool UnregisterBoneComponent(UNBoneComponent* Component);
	/** Unregister a cell root component. @return true if it was removed. */
	static bool UnregisterCellRootComponent(UNCellRootComponent* Component);
	/** Unregister a cell junction component. @return true if it was removed. */
	static bool UnregisterCellJunctionComponent(UNCellJunctionComponent* Component);
	/** Unregister an organ component. @return true if it was removed. */
	static bool UnregisterOrganComponent(UNOrganComponent* Organ);
	/** Unregister an operation and broadcast OnOperationStateChanged with Unregistered. @return true if it was removed. */
	static bool UnregisterOperation(UNProcGenOperation* Operation);
	/** Unregister a cell level instance. @return true if it was removed. */
	static bool UnregisterCellLevelInstance(ANCellLevelInstance* CellLevelInstance);

	/** @return All currently tracked operations. */
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
	/** Level instances keyed by operation ticket; ticket 0 denotes "any operation" in query helpers. */
	static TMap<uint32, TArray<ANCellLevelInstance*>> CellLevelInstances;
};