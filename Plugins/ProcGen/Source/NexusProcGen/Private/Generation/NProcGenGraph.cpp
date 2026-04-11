// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraph.h"

FNProcGenGraph::FNProcGenGraph(FNProcGenGraphNode* RootNodePtr)
{
	RootNode = RootNodePtr;
	Nodes.Add(RootNodePtr);
}

FNProcGenGraph::~FNProcGenGraph()
{
	const int NodeCount = Nodes.Num();
	for (int i = NodeCount - 1; i >= 0; i--)
	{
		delete Nodes[i];
	}
	Nodes.Empty();
	RootNode = nullptr;
}

void FNProcGenGraph::RegisterNode(FNProcGenGraphNode* Node)
{
	// TODO: Unique check overkill?
	Nodes.AddUnique(Node);
}
