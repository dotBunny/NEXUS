// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Async/TaskGraphInterfaces.h"

/**
 * Task-graph job that hands the junction pairings accepted by FNConnectJunctionsTask to the World Assembly
 * subsystem, and warms the project-wide default connector class so it is resident when they are built.
 *
 * Deliberately does not spawn anything. A connector spans two cells, and cells stream in asynchronously — at the
 * point this runs, neither endpoint's UNCellJunctionComponent exists, so there is nothing to connect and no way to
 * resolve the junction-level overrides that live on those components. The junctions report in as they begin play
 * and the subsystem builds each pairing once both ends have arrived.
 *
 * Runs on the game thread alongside FNSpawnCellProxiesTask: it touches the subsystem and the asset manager, and it
 * is cheap enough not to warrant a slice of its own.
 */
struct FNSpawnJunctionConnectorsTask
{
	explicit FNSpawnJunctionConnectorsTask(const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNSpawnJunctionConnectorsTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: registers every accepted pairing with the subsystem. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** Task-graph context supplying the accepted pairings and the target world. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};
