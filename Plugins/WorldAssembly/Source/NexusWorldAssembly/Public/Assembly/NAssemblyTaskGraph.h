// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NAssemblyTaskAnalytics.h"

struct FNStatusChannelUpdate;
class UNAssemblyOperation;
class FNAssemblyOperationContext;
class FNAssemblyTaskGraphContext;
class FNSpawnContext;
class FNVirtualWorldContext;

/**
 * Task-graph wrapper that drives a single UNAssemblyOperation through its graph-builder,
 * collection, and finalize stages.
 *
 * Construction builds (but does not dispatch) the graph; UnlockTasks kicks the dispatch and
 * the operation drains progress via GetTaskStatus. WaitForTasks blocks until every stage
 * finishes — intended for editor/test paths that need synchronous completion.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyTaskGraph
{
public:
	/**
	 * Creates the executable task-graph based on the provided Operation and initial Context.
	 * @param Operation The instigating generation operation which will own this graph and its execution.
	 * @param Context The world/generation context to use when building out the graph.
	 */
	explicit FNAssemblyTaskGraph(UNAssemblyOperation* Operation, FNAssemblyOperationContext* Context);

	/**
	 * @return (Completed, Total) task counts across all stages, for progress display.
	 * @remark Thread-safe to call from any thread once UnlockTasks() has dispatched the graph and before TearDownGraph()
	 *         runs: the AllTasks array is built during construction and never mutated thereafter, and FGraphEvent::IsComplete()
	 *         reads its completion state atomically. Calling this before UnlockTasks() (while the graph is still being
	 *         assembled) or concurrently with TearDownGraph() is unsafe — the array can be in flux. The returned snapshot
	 *         may already be stale by the time the caller inspects it, since tasks can complete between iteration steps;
	 *         use it for progress reporting, not for correctness decisions.
	 */
	FIntVector2 GetTaskStatus() const;

	/** @return true once UnlockTasks has dispatched the graph. */
	bool IsTasksUnlocked() const { return bTasksUnlocked; }

	/**
	 * Drain the most recent task-authored display message. Game thread only.
	 * @param OutMessage Receives the pending message when one is available.
	 * @return true if a new message was drained since the last call; false if nothing changed (or the graph context is gone).
	 */
	bool ConsumeStatusMessage(FString& OutMessage) const;

	/**
	 * Drain every per-stage status channel that changed since the last call. Game thread only.
	 * @param OutChanges Receives the current state of each dirtied channel, in stable open order.
	 * @return true if any channel was drained since the last call; false if nothing changed (or the graph context is gone).
	 */
	bool ConsumeChannelUpdates(TArray<FNStatusChannelUpdate>& OutChanges) const;

	/** Signal the spawn pass to stop producing new proxies at the next time-slice boundary. */
	void Cancel();

	/** Tear down the created objects in the graph in preparation for deletion. */
	void TearDownGraph();

	/** Dispatch all built tasks onto the task graph for execution. */
	void UnlockTasks();

	/** Block the calling thread until every stage finishes. */
	void WaitForTasks();
	
	TSharedRef<FNAssemblyTaskGraphContext> GetContext() const { return TaskGraphContextPtr.ToSharedRef(); }
	
	N_ASSEMBLY_ANALYTICS_SHARED_PTR
	
private:
	
	/** True once UnlockTasks has dispatched the graph for execution. */
	bool bTasksUnlocked = false;

	/** Terminal task that runs after every other stage completes. */
	FGraphEventRef FinalizeTask;

	/** Flat list of every task in the graph; used for status aggregation. */
	FGraphEventArray AllTasks;

	/** Pre-work on game thread tasks; kicks off before any per-organ work. */
	FGraphEventArray PreGameThreadTasks;
	
	/** Post pre-work game-thread tasks; gates the graph-builder batches. */
	FGraphEventArray ProcessInitialGameThreadTasks;

	/** Per-pass FNProcessPassTask wrappers; each entry holds a single event so the next pass can chain on it. */
	TArray<FGraphEventArray> ProcessPassTasks;

	/** Collection/pass tasks that gather results after graph construction. */
	FGraphEventArray CollectionTasks;

	/** Spawn-context preparation tasks that flatten collected graphs ready for proxy spawning. */
	FGraphEventArray SpawnContextTasks;

	/** Finalizer tasks that run immediately before FinalizeTask. */
	FGraphEventArray FinalizerTasks;

	/** Task-graph-wide context shared across all stages; held here so TearDownGraph() can null the world pointer. */
	TSharedPtr<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	/** Virtual-world collision snapshot; held here so TearDownGraph() can null the world pointer. */
	TSharedPtr<FNVirtualWorldContext> VirtualWorldContextPtr;

	/** Spawn context shared with the spawn tasks; held here so Cancel() can set its cancellation flag. */
	TSharedPtr<FNSpawnContext> SpawnContextPtr;
};