// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/INAssemblyOperationOwner.h"
#include "Engine/TimerHandle.h"
#include "NEditorUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "Cell/NCellJunctionConnection.h"
#include "Developer/NDebugActor.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NWorldAssemblyEditorSubsystem.generated.h"

class ANCellProxy;
class UNAssemblyOperation;
struct FPropertyChangedEvent;

/**
 * What the Operations panel reports about an assembly run once it has finished.
 *
 * A copy of the parts of FNAssemblyOperationResult worth showing after the fact, plus the report the run wrote.
 * Copied rather than held by reference: the operation is torn down moments after it finishes, taking its result and
 * its task-graph context with it, and the panel outlives both.
 */
struct FNAssemblyRunSummary
{
	/** Ticket of the run this summarizes, used to tell a finished run's teardown from a cancellation. */
	int32 Ticket = INDEX_NONE;

	/** true if the run was cancelled before finishing, in which case it carries a title and nothing else. */
	bool bCancelled = false;

	/** true if the run completed successfully. */
	bool bSuccess = false;

	/** true if the run completed but produced one or more warnings. */
	bool bWarning = false;

	/** Short result title, the same one the completion toast leads with. */
	FText Title;

	/**
	 * Detailed result message.
	 * @note Carries the cell count and duration in prose, which is why neither is copied out separately — the panel
	 *       shows this line rather than restating the same two numbers above it.
	 */
	FText Message;

	/** Absolute path to the report the run wrote, or empty when it wrote none. */
	FString ReportFilePath;
};

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

		// Coalesce any world changes flagged since the last tick into a single in-place rebuild of the visualizer.
		TickCollisionVisualizer();

		LastFrameNumberWeTicked = GFrameCounter;
	}

	virtual bool IsTickable() const override
	{
		if (LastFrameNumberWeTicked == GFrameCounter ||
			FNEditorUtils::IsEditorShuttingDown())
		{
			return false;
		}

		// Tick while operations run, while an auto-assembly loop is waiting between runs so the toolbar countdown
		// bar keeps advancing, or while a world edit has left the collision visualizer needing a rebuild.
		return HasKnownOperation() || (bAutoAssemblyLoopActive && AutoAssemblyTimerHandle.IsValid()) ||
			bCollisionVisualizerDirty;
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

	/**
	 * @return The most recent assembly run, or unset when none has been started since the editor opened.
	 * @note Covers cancellations as well as completions — a cancelled run replaces whatever came before it with a
	 *       summary carrying only bCancelled and a title, so the panel never reports stale numbers as if they were
	 *       the outcome of the run the user just stopped.
	 */
	const TOptional<FNAssemblyRunSummary>& GetLastRunSummary() const { return LastRunSummary; }

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

	/**
	 * Builds — or refreshes in place — the world-collision visualizer: a single merged ANDebugActor whose mesh is the
	 * level's baked collision pool as FNWorldCollisionPreview reads it, shaded with
	 * UNWorldAssemblyEditorSettings::CollisionVisualizerMaterial.
	 *
	 * When no visualizer is alive this spawns one and starts listening for world changes; when one already exists its
	 * geometry is swapped in place, preserving actor identity and selection. Diagnostic — the actor is transient and
	 * is not saved with the level.
	 * @param World World to read the pool from and to spawn the visualizer into. Must be valid.
	 * @return The live visualizer actor, or nullptr when the level has nothing baked to show.
	 * @note Owned here rather than by UNWorldAssemblyEdMode, which held it until this moved. The mode is only one of
	 *       the places that offers the visualizer — ANWorldCollisionCacheActor's details panel is another, and that
	 *       panel is reachable from the Outliner whether or not the mode is up. A mode-owned visualizer could not
	 *       exist while the mode was closed, which made every button outside the mode a no-op.
	 */
	ANDebugActor* CreateCollisionVisualizer(UWorld* World);

	/** Destroy the live world-collision visualizer, if there is one. No-op when there is not. */
	void DestroyCollisionVisualizer();

	/** @return true while a world-collision visualizer actor is alive. */
	bool HasCollisionVisualizer() const { return CollisionVisualizer != nullptr; }

	/** @return The live world-collision visualizer, or nullptr when none is alive. */
	ANDebugActor* GetCollisionVisualizer() const { return CollisionVisualizer; }

	/**
	 * Fires whenever the visualizer is spawned or destroyed.
	 * @note What lets a UI showing its state track a toggle it did not make itself — the ed mode rail and the cache
	 *       actor's details panel both drive the same visualizer, and either can act while the other is on screen.
	 */
	FSimpleMulticastDelegate OnCollisionVisualizerChanged;

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

	/** Rebuild the visualizer in place when a world change has flagged it. Called every Tick; no-op while clean. */
	void TickCollisionVisualizer();

	/** Subscribe to the editor world-change delegates that drive live visualizer refreshes. Called when one is spawned. */
	void BindWorldChangeDelegates();

	/** Unsubscribe from the editor world-change delegates. Called when the visualizer is destroyed. */
	void UnbindWorldChangeDelegates();

	/** Flag the visualizer for a rebuild on the next Tick. Bursts of changes coalesce into a single rebuild. */
	void MarkCollisionVisualizerDirty() { bCollisionVisualizerDirty = true; }

	/**
	 * @return true when a change to Actor could alter what the visualizer draws — i.e. Actor passes the world
	 *         collision filter. Always false while no visualizer is alive.
	 */
	bool ShouldRebuildForActor(const AActor* Actor) const;

	/** @return The actor affected by a change delegate payload — the object itself, or its owner when it is a component. */
	static AActor* ResolveAffectedActor(UObject* Object);

	/** Delegate: a relevant actor was added to the level — flag a refresh. */
	void OnLevelActorAdded(AActor* Actor);

	/**
	 * Delegate: an actor was removed from the level — clear our state when it was the visualizer, flag a refresh
	 * otherwise.
	 * @note Every deletion refreshes, not just a relevant one: what the visualizer draws comes from the level's baked
	 *       pool, and FNWorldCollisionPreview decides for itself whether that pool still matches the world.
	 */
	void OnLevelActorDeleted(AActor* Actor);

	/** Delegate: a transform gizmo drag ended on Object — flag a refresh when it is relevant. */
	void OnObjectMoved(UObject& Object);

	/** Delegate: a finalized (non-interactive) property edit landed on Object — flag a refresh when it is relevant. */
	void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	/** Delegate: an undo/redo transaction completed — geometry can't be cheaply diffed, so always flag a refresh. */
	void OnUndoRedo();

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

	/**
	 * The live world-collision visualizer, or nullptr when none is spawned.
	 * @note A UPROPERTY because that is what roots it — the actor is transient, so its world's actor list is not
	 *       enough to keep it from being collected.
	 */
	UPROPERTY()
	TObjectPtr<ANDebugActor> CollisionVisualizer;

	/** Set by the world-change delegates when the visualizer needs rebuilding; consumed (and cleared) in Tick. */
	bool bCollisionVisualizerDirty = false;

	FDelegateHandle OnLevelActorAddedHandle;
	FDelegateHandle OnLevelActorDeletedHandle;
	FDelegateHandle OnObjectMovedHandle;
	FDelegateHandle OnObjectPropertyChangedHandle;
	FDelegateHandle OnUndoRedoHandle;

public:
	/** One operation's accepted junction pairings, retained so the ed mode can draw the routes it proved clear. */
	struct FNGeneratedConnections
	{
		/**
		 * World the pairings were generated into.
		 * @note Held because FNCellJunctionConnection is a plain struct of world-space points with no object
		 *       references — unlike the proxy entries, whose weak pointers self-null when their world goes away,
		 *       these would happily outlive their world and draw into an unrelated one.
		 */
		TWeakObjectPtr<UWorld> World;

		/** The pairings, copied out of the task-graph context before it is torn down. */
		TArray<FNCellJunctionConnection> Connections;
	};

	/**
	 * @return Accepted junction pairings from every completed operation, keyed by operation ticket.
	 * @note Editor-side only. The routes live on the task-graph context, which is destroyed with the operation, and
	 *       the runtime subsystem never receives them for an editor preview — FNSpawnJunctionConnectorsTask skips
	 *       registration when the operation creates no level instances, which is the editor default.
	 */
	const TMap<int32, FNGeneratedConnections>& GetGeneratedConnections() const { return ConnectionMap; }

private:

	/** Index from operation ticket to the proxies spawned by that operation, for fast per-operation cleanup. */
	TMap<int32, TArray<TObjectPtr<ANCellProxy>>> ProxyMap;

	/**
	 * Index from operation ticket to that operation's accepted junction pairings; the visual counterpart to ProxyMap,
	 * cleared on the same paths.
	 * @note Not a UPROPERTY: FNCellJunctionConnection holds no object references, and the world it carries is a weak
	 *       pointer, so there is nothing here for reflection to keep alive.
	 */
	TMap<int32, FNGeneratedConnections> ConnectionMap;

	/**
	 * The last finished run's summary, for the Operations panel to report while it sits idle.
	 * @note Not a UPROPERTY: everything in FNAssemblyRunSummary is plain text and numbers, so there is nothing here
	 *       for reflection to keep alive.
	 */
	TOptional<FNAssemblyRunSummary> LastRunSummary;

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