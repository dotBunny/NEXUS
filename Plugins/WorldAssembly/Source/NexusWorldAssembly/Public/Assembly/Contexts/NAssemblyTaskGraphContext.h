// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyOperationSettings.h"
#include "Cell/NCellProxy.h"
#include "Assembly/Graph/NAssemblyGraph.h"

/**
 * Execution-time context shared across all tasks in a single FNAssemblyTaskGraph run.
 *
 * Collects the graph results (per-organ graphs, spawned proxies) plus the settings/ticket
 * each task needs to operate without reaching back into the operation.
 */
class FNAssemblyTaskGraphContext
{
public:
	/** The UWorld to target when creating or querying from the FNAssemblyTaskGraph. */
	TObjectPtr<UWorld> TargetWorld;

	/** The proxies that were created from the evaluated task graph. */
	TArray<TObjectPtr<ANCellProxy>> CreatedProxies;

	/** The operation identifier for this graph. */
	uint32 OperationTicket;

	/** Operation-specific settings effecting the behavior of some bits of the graph. */
	FNAssemblyOperationSettings OperationSettings;

	/** Transfer ownership of a built organ-graph into this context. */
	void TakeGraph(TUniquePtr<FNAssemblyGraph> Graph)
	{
		FScopeLock Lock(&TakeGraphMutex);
		Graphs.Add(MoveTemp(Graph));
	}

	/** Append additional context tags to this context's accumulated set. */
	void AddContextTags(const FGameplayTagContainer& NewContextTags)
	{
		ContextTags.AppendTags(NewContextTags);
	}

	/** Built per-organ graphs owned by this context. */
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	/** Context tags accumulated across the graphs built into this context. */
	FGameplayTagContainer ContextTags;

	/**
	 * @param OutputWorld World to target when spawning proxies.
	 * @param OperationTicket Identifier of the operation that owns this context.
	 * @param Settings Operation-wide settings forwarded to dependent tasks.
	 */
	explicit FNAssemblyTaskGraphContext(UWorld* OutputWorld, const uint32& OperationTicket, const FNAssemblyOperationSettings& Settings);

	
	/** Path the operation's report is written to, when report output is enabled. */
	FString ReportFilePath;

private:
	FCriticalSection TakeGraphMutex;
};
