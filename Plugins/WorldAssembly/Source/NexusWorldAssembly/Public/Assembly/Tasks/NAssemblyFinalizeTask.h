// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Assembly/NAssemblyTaskAnalytics.h"

class UNAssemblyOperation;

/**
 * Terminal task that notifies the owning UNAssemblyOperation the graph has drained.
 *
 * Runs on the game thread so it can safely fire operation completion delegates and unlink
 * the task-graph context from the operation.
 */
struct FNAssemblyFinalizeTask
{
public:

	explicit FNAssemblyFinalizeTask(UNAssemblyOperation* TargetOperation, const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNWorldAssemblyFinalizeTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: fires the operation's completion path on the game thread. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/**
	 * Operation whose completion this task signals.
	 * @note Weak because the task struct is invisible to GC; a cancel can tear the operation down
	 * (RemoveFromRoot + MarkAsGarbage) and a GC purge can free it while the graph is still draining.
	 */
	TWeakObjectPtr<UNAssemblyOperation> Operation;

	/** Top-level task-graph context containing the produced graphs and spawned proxies. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};
