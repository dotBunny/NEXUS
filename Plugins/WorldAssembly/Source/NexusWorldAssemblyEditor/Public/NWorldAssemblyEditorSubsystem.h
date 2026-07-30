// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/INAssemblyOperationOwner.h"
#include "Engine/TimerHandle.h"
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

		// While waiting between auto-assembly runs there is no live operation, so drive the toolbar progress
		// bar from the inter-run timer's countdown instead (no-op when no loop is waiting).
		UpdateAutoAssemblyCountdownBar();

		LastFrameNumberWeTicked = GFrameCounter;
	}

	virtual bool IsTickable() const override
	{
		if (LastFrameNumberWeTicked == GFrameCounter ||
			FNEditorUtils::IsEditorShuttingDown())
		{
			return false;
		}

		// Tick while operations run, or while an auto-assembly loop is waiting between runs so the toolbar
		// countdown bar keeps advancing.
		return HasKnownOperation() || (bAutoAssemblyLoopActive && AutoAssemblyTimerHandle.IsValid());
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

	/** @return true while an auto-assembly loop is engaged — covers both the running operation and the wait between runs. */
	bool IsAutoAssemblyLoopActive() const { return bAutoAssemblyLoopActive; }

	/** Engage the auto-assembly loop so the Quick Assembly button stays in its cancel state across inter-run waits. */
	void BeginAutoAssemblyLoop();

	/**
	 * Disengage the auto-assembly loop, clearing any pending inter-run timer and (when idle) the toolbar progress bar.
	 * @param bEmitSummary When true, and at least one run was accumulated, surface the pass/warn/fail summary toast
	 *        before discarding the accumulator. Pass true only for user-initiated stops (toolbar cancel, toggling
	 *        Auto Assembly off); leave false for environment-driven stops (can't-run, PIE, map-load, shutdown).
	 */
	void StopAutoAssemblyLoop(bool bEmitSummary = false);

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
	/**
	 * Running pass/warn/fail tally accumulated across the runs of a single auto-assembly loop session, so the loop can
	 * report one summary toast on stop instead of one toast per run.
	 */
	struct FNAutoAssemblySummary
	{
		/** Number of completed runs folded into this summary. */
		int32 TotalRuns = 0;
		/** Runs that succeeded with no warnings. */
		int32 PassCount = 0;
		/** Runs that completed with one or more warnings (takes precedence over success, matching the toast icon logic). */
		int32 WarningCount = 0;
		/** Runs that failed. */
		int32 FailCount = 0;
		/** Total cells created across all accumulated runs. */
		int32 TotalCreatedCells = 0;
		/** Total wall-clock time across all accumulated runs, in milliseconds. */
		double TotalDurationMs = 0.0;

		void Reset() { *this = FNAutoAssemblySummary(); }
	};

	/** Fold a finished loop run's result into AutoAssemblySummary using the same warn-over-success precedence as the per-op toast. */
	void AccumulateAutoAssemblyResult(const FNAssemblyOperationResult& Result);

	/** Surface the accumulated pass/warn/fail summary (with totals) as a single toast. Game thread only. */
	void ShowAutoAssemblySummaryToast() const;

	/** Schedule the next auto-assembly run using the live QuickAssemblyAutoAssemblyTimer value. */
	void ScheduleNextAutoAssembly();

	/** Inter-run timer callback: starts the next auto-assembly run, or stops the loop if it can no longer run. */
	void OnAutoAssemblyTimerElapsed();

	/** Push the inter-run countdown (0..1) onto the toolbar progress bar while waiting between auto-assembly runs. Game thread only. */
	void UpdateAutoAssemblyCountdownBar();

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

	/** true while an auto-assembly loop is engaged (running operation or waiting on AutoAssemblyTimerHandle). */
	bool bAutoAssemblyLoopActive = false;

	/** Handle for the inter-run delay timer scheduled on the editor timer manager. */
	FTimerHandle AutoAssemblyTimerHandle;

	/** Pass/warn/fail tally for the current auto-assembly loop session; reset when a loop begins and after each stop. */
	FNAutoAssemblySummary AutoAssemblySummary;
};