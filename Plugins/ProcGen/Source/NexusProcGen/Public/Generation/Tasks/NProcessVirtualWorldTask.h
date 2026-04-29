// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NVirtualWorldContext.h"

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
	explicit FNProcessVirtualWorldTask(const TSharedPtr<FNVirtualWorldContext>& VirtualWorldContextPtr
		N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR)
	: VirtualWorldContextPtr(VirtualWorldContextPtr.ToSharedRef()) N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER { }

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcessVirtualWorldTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: processes the virtual-world context's collision snapshot. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Virtual-world context being processed; populated by FNCreateVirtualWorldTask. */
	TSharedRef<FNVirtualWorldContext> VirtualWorldContextPtr;

	N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF
};
