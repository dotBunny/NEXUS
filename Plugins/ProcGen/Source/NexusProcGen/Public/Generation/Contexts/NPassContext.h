// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Graph/NProcGenGraph.h"

/**
 * Shared collect-pass context — fans-in completed organ graphs from every parallel builder task
 * so the subsequent collect/finalize stages have a single place to walk.
 */
class FNPassContext
{
public:
	/** Transfer ownership of a completed graph into this pass context. */
	void TakeGraph(TUniquePtr<FNProcGenGraph> Graph)
	{
		Graphs.Add(MoveTemp(Graph));
	}

	/** Graphs accumulated from this collect pass. */
	TArray<TUniquePtr<FNProcGenGraph>> Graphs;
};
