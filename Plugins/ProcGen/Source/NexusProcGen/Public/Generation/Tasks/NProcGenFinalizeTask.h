// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "Generation/NProcGenTaskAnalytics.h"

class UNProcGenOperation;

/**
 * Terminal task that notifies the owning UNProcGenOperation the graph has drained.
 *
 * Runs on the game thread so it can safely fire operation completion delegates and unlink
 * the task-graph context from the operation.
 */
struct FNProcGenFinalizeTask
{
public:

	explicit FNProcGenFinalizeTask(UNProcGenOperation* TargetOperation, const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr,
		const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenOperationFinalizeTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: fires the operation's completion path on the game thread. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Operation whose completion this task signals. */
	UNProcGenOperation* Operation;

	/** Top-level task-graph context containing the produced graphs and spawned proxies. */
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;
	
	TSharedRef<FNProcGenTaskAnalytics> AnalyticsPtr;
};
