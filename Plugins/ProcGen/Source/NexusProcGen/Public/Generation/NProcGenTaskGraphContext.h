// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NProcGenOperationSettings.h"
#include "Cell/NCellProxy.h"
#include "Generation/Graph/NProcGenGraph.h"

class FNProcGenTaskGraphContext
{
public:
	/**
	 * The UWorld to target when creating or querying from the NProcGenTaskGraph.
	 */
	UWorld* TargetWorld;

	/**
	 * The proxies that were created from the evaluated task graph.
	 */
	TArray<ANCellProxy*> CreatedProxies;

	/**
	 * The operation identifier for this graph.
	 */
	uint32 OperationTicket;
	
	/**
	 * Operation-specific settings effecting the behavior of some bits of the graph.
	 */
	FNProcGenOperationSettings OperationSettings;
	
	void TakeGraph(TUniquePtr<FNProcGenGraph> Graph)
	{
		Graphs.Add(MoveTemp(Graph));
	}
	
	TArray<TUniquePtr<FNProcGenGraph>> Graphs;
	
	explicit FNProcGenTaskGraphContext(UWorld* OutputWorld, const uint32& OperationTicket, const FNProcGenOperationSettings& Settings);
};
