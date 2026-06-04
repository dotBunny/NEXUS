// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Assembly/NAssemblyOperationSettings.h"
#include "Developer/NReport.h"
#include "Assembly/Data/NWorldBoneData.h"
#include "Assembly/Data/NWorldOrganData.h"
#include "Organ/NBoneComponent.h"

class UNOrganComponent;

/**
 * Game-thread context of an entire generation process.
 *
 * Aggregates every input organ, its derived per-organ and per-bone state, and the computed
 * generation ordering. Populated by UNAssemblyOperation while the operation is being configured,
 * then locked once preprocessing completes and used by task-graph stages as the source of truth.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyOperationContext
{
	friend class UNAssemblyOperation;
public:
	/** Constructs the context for the operation identified by the given ticket. */
	explicit FNAssemblyOperationContext(int32 NewOperationTicket);

	/** Organ components fed into the operation prior to LockAndPreprocess. */
	TArray<TObjectPtr<UNOrganComponent>> InputComponents;

	/** Per-organ derived state (bounds, bone references, assigned tissues, etc.). */
	TMap<TObjectPtr<UNOrganComponent>, FNWorldOrganData> OrganData;

	/** Per-bone derived state keyed by the bone component it was computed from. */
	TMap<TObjectPtr<UNBoneComponent>, FNWorldBoneData> BoneData;

	/** Reverse index from organ to the bone components that belong to it. */
	TMap<TObjectPtr<UNOrganComponent>, TArray<TObjectPtr<UNBoneComponent>>> ComponentBoneMap;

	/** Organ components grouped into topologically-ordered batches that may run in parallel per step. */
	TArray<TArray<TObjectPtr<UNOrganComponent>>> GenerationOrder;

	/** Spatial bounds of every input organ; forwarded to FNVirtualWorldContext to scope collision gathering. */
	TArray<FBoxSphereBounds> Bounds;
	
	/**
	 * Register an organ component as a generation input.
	 * @return true if the component was accepted; false if the context is already locked or the component was rejected.
	 */
	bool AddOrganComponent(UNOrganComponent* Component);

	/** Store the operation-wide settings (seed, replication flags, display name) on this context. */
	void SetOperationSettings(const FNAssemblyOperationSettings& InSettings);

	/** @return true once LockAndPreprocess has finished — no further mutation should occur. */
	bool IsLocked() const { return bIsLocked; }

	/** Finalize the context: compute per-organ/per-bone state, build the generation ordering, create world collision structure, and mark locked. */
	void LockAndPreprocess(UWorld* World);
	
#if !UE_BUILD_SHIPPING
	/**
	 * Appends a summary of this context's organs, bones, and generation order to the supplied report (non-shipping builds only).
	 * @param Report The report to append to.
	 * @param bBuildTissues When true, also include per-tissue detail in the summary.
	 */
	void AddToReport(FNReport* Report, bool bBuildTissues = false);
#endif // !UE_BUILD_SHIPPING

	/** Clear all derived state and return the context to an unlocked, empty state. */
	void ResetContext();

	/** Assign the world that will host the generated content. */
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }

	/** @return The world that will host the generated content. */
	UWorld* GetTargetWorld() const { return TargetWorld; }

	/** @return The ticket of the UNAssemblyOperation that owns this context. */
	int32 GetOperationTicket() const { return OperationTicket; }

	/** @return The operation-wide settings snapshot. */
	const FNAssemblyOperationSettings& GetOperationSettings() { return OperationSettings; }

private:

	/** The identifier of the UNAssemblyOperation that owns this context. */
	int32 OperationTicket;

	/** Operation-wide settings snapshot, captured before preprocessing. */
	FNAssemblyOperationSettings OperationSettings;

	/** True once LockAndPreprocess has completed; guards against further mutation. */
	bool bIsLocked = false;

	/** World that will host the generated content. */
	TObjectPtr<UWorld> TargetWorld = nullptr;
};
