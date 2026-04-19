// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Graph/NProcGenGraph.h"

class FNCollectPassContext
{
public:
	void TakeGraph(TUniquePtr<FNProcGenGraph> Graph)
	{
		Graphs.Add(MoveTemp(Graph));
	}
	
	TArray<TUniquePtr<FNProcGenGraph>> Graphs;
};
