// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NProcGenOperationSettings.h"
#include "Cell/NCellProxy.h"
#include "Generation/Graph/NProcGenGraph.h"

/**
 * Execution-time context shared across all tasks in a single FNProcGenTaskGraph run.
 *
 * Collects the graph results (per-organ graphs, spawned proxies) plus the settings/ticket
 * each task needs to operate without reaching back into the operation.
 */
class FNProcGenTaskGraphContext
{
public:
	/** The UWorld to target when creating or querying from the NProcGenTaskGraph. */
	TObjectPtr<UWorld> TargetWorld;

	/** The proxies that were created from the evaluated task graph. */
	TArray<TObjectPtr<ANCellProxy>> CreatedProxies;

	/** The operation identifier for this graph. */
	uint32 OperationTicket;

	/** Operation-specific settings effecting the behavior of some bits of the graph. */
	FNProcGenOperationSettings OperationSettings;

	/** Transfer ownership of a built organ-graph into this context. */
	void TakeGraph(TUniquePtr<FNProcGenGraph> Graph)
	{
		Graphs.Add(MoveTemp(Graph));
	}

	/** Built per-organ graphs owned by this context. */
	TArray<TUniquePtr<FNProcGenGraph>> Graphs;

	/**
	 * @param OutputWorld World to target when spawning proxies.
	 * @param OperationTicket Identifier of the operation that owns this context.
	 * @param Settings Operation-wide settings forwarded to dependent tasks.
	 */
	explicit FNProcGenTaskGraphContext(UWorld* OutputWorld, const uint32& OperationTicket, const FNProcGenOperationSettings& Settings);
};
