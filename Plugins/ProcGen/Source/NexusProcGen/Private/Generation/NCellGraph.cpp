// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NCellGraph.h"



FNCellGraph::FNCellGraph(FNCellGraphNode* RootNodePtr)
{
	RootNode = RootNodePtr;
	Nodes.Add(RootNodePtr);
}

FNCellGraph::~FNCellGraph()
{
	const int NodeCount = Nodes.Num();
	for (int i = NodeCount - 1; i >= 0; i--)
	{
		delete Nodes[i];
	}
	Nodes.Empty();
	RootNode = nullptr;
}
