// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenTaskAnalytics.h"

class UNProcGenOperation;
class FNProcGenOperationContext;

/**
 * Task-graph wrapper that drives a single UNProcGenOperation through its graph-builder,
 * collection, and finalize stages.
 *
 * Construction builds (but does not dispatch) the graph; UnlockTasks kicks the dispatch and
 * the operation drains progress via GetTaskStatus. WaitForTasks blocks until every stage
 * finishes — intended for editor/test paths that need synchronous completion.
 */
class NEXUSPROCGEN_API FNProcGenTaskGraph
{
public:
	/**
	 * Creates the executable task-graph based on the provided Operation and initial Context.
	 * @param Operation The instigating generation operation which will own this graph and its execution.
	 * @param Context The world/generation context to use when building out the graph.
	 */
	explicit FNProcGenTaskGraph(UNProcGenOperation* Operation, FNProcGenOperationContext* Context);

	/** @return (Completed, Total) task counts across all stages, for progress display. */
	FIntVector2 GetTaskStatus() const;

	/** @return true once UnlockTasks has dispatched the graph. */
	bool IsTasksUnlocked() const { return bTasksUnlocked; }

	/** Tear down the created objects in the graph in preparation for deletion. */
	void TearDownGraph();

	/** Dispatch all built tasks onto the task graph for execution. */
	void UnlockTasks();

	/** Block the calling thread until every stage finishes. */
	void WaitForTasks();
	
	N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_PTR
	
private:
	
	/** True once UnlockTasks has dispatched the graph for execution. */
	bool bTasksUnlocked = false;

	/** Terminal task that runs after every other stage completes. */
	FGraphEventRef FinalizeTask;

	/** Flat list of every task in the graph; used for status aggregation. */
	FGraphEventArray AllTasks;
	
	FGraphEventArray Step0Tasks;
	FGraphEventArray Step1Tasks;

	/** Graph-builder tasks grouped per generation-order batch. */
	TArray<FGraphEventArray> GraphBuilderTasks;

	/** Collection/pass tasks that gather results after graph construction. */
	FGraphEventArray CollectionTasks;
	
	FGraphEventArray SpawnContextTasks;

	/** Finalizer tasks that run immediately before FinalizeTask. */
	FGraphEventArray FinalizerTasks;
};