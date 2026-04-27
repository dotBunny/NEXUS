// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorUtils.h"
#include "NProcGenOperationSettings.h"
#include "Generation/Data/NBoneLockedData.h"
#include "Generation/Data/NOrganLockedData.h"
#include "Organ/NBoneComponent.h"
#include "Types/NRawMesh.h"

class UNOrganComponent;

/**
 * Game-thread context of an entire generation process.
 *
 * Aggregates every input organ, its derived per-organ and per-bone state, and the computed
 * generation ordering. Populated by UNProcGenOperation while the operation is being configured,
 * then locked once preprocessing completes and used by task-graph stages as the source of truth.
 */
class NEXUSPROCGEN_API FNProcGenOperationContext
{
	friend class UNProcGenOperation;
public:
	explicit FNProcGenOperationContext(uint32 NewOperationTicket);

	/** Organ components fed into the operation prior to LockAndPreprocess. */
	TArray<UNOrganComponent*> InputComponents;

	/** Per-organ derived state (bounds, bone references, assigned tissues, etc.). */
	TMap<UNOrganComponent*, FNOrganLockedData> OrganContext;

	/** Per-bone derived state keyed by the bone component it was computed from. */
	TMap<UNBoneComponent*, FNBoneLockedData> BoneContext;

	/** Reverse index from organ to the bone components that belong to it. */
	TMap<UNOrganComponent*, TArray<UNBoneComponent*>> ComponentBoneMap;

	/** Organ components grouped into topologically-ordered batches that may run in parallel per step. */
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	
	TArray<FBoxSphereBounds> Bounds;
	
	/**
	 * Register an organ component as a generation input.
	 * @return true if the component was accepted; false if the context is already locked or the component was rejected.
	 */
	bool AddOrganComponent(UNOrganComponent* Component);

	/** Store the operation-wide settings (seed, replication flags, display name) on this context. */
	void SetOperationSettings(const FNProcGenOperationSettings& InSettings);

	/** @return true once LockAndPreprocess has finished — no further mutation should occur. */
	bool IsLocked() const { return bIsLocked; }

	/** Finalize the context: compute per-organ/per-bone state, build the generation ordering, create world collision structure, and mark locked. */
	void LockAndPreprocess(UWorld* World);

	/**
	 * Dump a human-readable summary of the context to the log.
	 * @param bBuildTissues When true, additionally walks and prints the flattened tissue/cell maps.
	 */
	void OutputToLog(bool bBuildTissues = false);

	/** Clear all derived state and return the context to an unlocked, empty state. */
	void ResetContext();

	/** Assign the world that will host the generated content. */
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }

	/** @return The world that will host the generated content. */
	UWorld* GetTargetWorld() const { return TargetWorld; }

	/** @return The ticket of the UNProcGenOperation that owns this context. */
	uint32 GetOperationTicket() const { return OperationTicket; }

	/** @return The operation-wide settings snapshot. */
	const FNProcGenOperationSettings& GetOperationSettings() { return OperationSettings; }

private:

	/** The identifier of the UNProcGenOperation that owns this context. */
	uint32 OperationTicket;

	/** Operation-wide settings snapshot, captured before preprocessing. */
	FNProcGenOperationSettings OperationSettings;

	/** True once LockAndPreprocess has completed; guards against further mutation. */
	bool bIsLocked = false;

	/** World that will host the generated content. */
	UWorld* TargetWorld = nullptr;
};
