// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/INAssemblyOperationOwner.h"
#include "NEditorUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NWorldAssemblyEditorSubsystem.generated.h"

class ANCellProxy;
class UNAssemblyOperation;

/**
 * Editor-side counterpart to UNWorldAssemblySubsystem: hosts World Assembly operations run from the editor UI.
 *
 * Drives operation ticking, tracks spawned cell proxies so they survive editor GC, and cleans
 * them up at PIE/map-load boundaries since the proxies are transient. Acts as the operation
 * owner for editor-initiated generation flows.
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNWorldAssemblyEditorSubsystem : public UEditorSubsystem, public FTickableGameObject, public INAssemblyOperationOwner
{
	GENERATED_BODY()
	N_EDITOR_TICKABLE_SUBSYSTEM(UNWorldAssemblyEditorSubsystem)

	//~UEditorSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//End UEditorSubsystem

	//~FTickableGameObject
	virtual void Tick(float DeltaTime) override
	{
		for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
		{
			KnownOperations[i]->Tick();
		}
		LastFrameNumberWeTicked = GFrameCounter;
	}

	virtual bool IsTickable() const override
	{
		if (!HasKnownOperation() ||
			LastFrameNumberWeTicked == GFrameCounter ||
			FNEditorUtils::IsEditorShuttingDown())
		{
			return false;
		}

		return true;
	}
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNWorldAssemblyEditorSubsystem, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override{ return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	//End FTickableGameObject

	//~INWorldAssemblyOperationOwner
	virtual void StartOperation(UNAssemblyOperation* Operation) override;
	virtual void OnOperationFinished(UNAssemblyOperation* Operation, TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext) override;
	virtual void OnOperationDestroyed(UNAssemblyOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return FNEditorUtils::GetCurrentWorld(); };
	//End INWorldAssemblyOperationOwner

	/**
	 * Forwards an operation's combined task + sub-channel progress to the Quick Assembly toolbar progress bar.
	 * Bound to UNAssemblyOperation::OnPercentageChanged for the quick-assembly operation in StartQuickAssembly.
	 */
	UFUNCTION()
	void OnQuickAssemblyProgressChanged(float Progress);

	/** @return true if at least one operation is currently tracked by this subsystem. */
	bool HasKnownOperation() const { return !KnownOperations.IsEmpty(); }

	/** @return true if there are cell proxies currently alive from a generation pass. */
	bool HasGeneratedCellProxies() const { return !KnownProxies.IsEmpty(); }

	/** @return true if proxies spawned by the given operation are still tracked and could be cleared. */
	bool HasGeneratedProxies(const int32& OperationTicket) const { return ProxyMap.Contains(OperationTicket); }

	/** Destroy every known proxy and drop tracking. */
	void ClearAllProxies();

	/** Destroy the proxies (and their level instances) associated with the given operation. */
	void ClearGenerated(const int32& OperationTicket);

	/** Destroy only the proxy actors associated with the given operation, leaving level instances alone. */
	void ClearGeneratedProxies(const int32& OperationTicket);

	/** Load the level instances for every tracked proxy. */
	void LoadAllGeneratedProxies();

	/** Load the level instances for proxies from the given operation. */
	void LoadGeneratedProxies(const int32& OperationTicket);

	/** Unload the level instances for every tracked proxy. */
	void UnloadAllGeneratedProxies();

	/** Unload the level instances for proxies from the given operation. */
	void UnloadGeneratedProxies(const int32& OperationTicket);

protected:
	/** Editor callback: drops proxies before PIE starts so transient actors don't leak into play. */
	void OnPreBeginPIE(bool bArg);

	/** Editor callback: drops proxies at map-load time. */
	void OnMapLoad(const FString& String, FCanLoadMap& CanLoadMap);

private:
	/** Operations currently owned by this subsystem. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	UPROPERTY()
	TArray<TObjectPtr<UNAssemblyOperation>> KnownOperations;
	
	/** Used to track potential operations that will cache data, so that we can clear it. **/
	TArray<int32> CachedOperationTickets;

	/**
	 * An array of all known proxies that have been put into the level, this ensures that they are not
	 * garbage collected and that their spawned level instances are protected as well. However, because
	 * they are marked transient we need to manage their cleanup explicitly when going into things like PIE
	 * or transitioning to new levels.
	 */
	UPROPERTY()
	TArray<TObjectPtr<ANCellProxy>> KnownProxies;

	/** Index from operation ticket to the proxies spawned by that operation, for fast per-operation cleanup. */
	TMap<int32, TArray<TObjectPtr<ANCellProxy>>> ProxyMap;

	/** Frame number of the last Tick — guards against re-entrant ticking from multiple callers. */
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

	/** Handle for the map-load delegate subscription. */
	FDelegateHandle OnMapLoadHandle;

	/** Handle for the PreBeginPIE delegate subscription. */
	FDelegateHandle PreBeginPIEHandle;
};