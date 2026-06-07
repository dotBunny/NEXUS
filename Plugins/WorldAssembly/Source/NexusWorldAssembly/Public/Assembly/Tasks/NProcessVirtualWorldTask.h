// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"

class FNAssemblyTaskGraphContext;

/**
 * Task-graph job that prepares the previously-collected world collision snapshot for use by the
 * organ-graph builder.
 *
 * Runs on any worker thread once FNCreateVirtualWorldTask has finished gathering source actors,
 * and transforms the gathered meshes/locations/rotations into the form expected by collision
 * checks performed during graph expansion.
 */
class FNProcessVirtualWorldTask
{
public:
	explicit FNProcessVirtualWorldTask(const TSharedPtr<FNVirtualWorldContext>& VirtualWorldContextPtr,
		const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: VirtualWorldContextPtr(VirtualWorldContextPtr.ToSharedRef())
	, TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER { }

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcessVirtualWorldTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyNormalThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: processes the virtual-world context's collision snapshot. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Virtual-world context being processed; populated by FNCreateVirtualWorldTask. */
	TSharedRef<FNVirtualWorldContext> VirtualWorldContextPtr;

	/** Task-graph-wide context used to publish the phase status message as organ building begins. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};
