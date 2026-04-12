// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraph.h"

FNProcGenGraph::FNProcGenGraph(FNProcGenGraphNode* RootNodePtr) : RootNode(RootNodePtr)
{
	Nodes.Add(RootNodePtr);
}

FNProcGenGraph::~FNProcGenGraph()
{
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory
	const int NodeCount = Nodes.Num();
	for (int i = NodeCount - 1; i >= 0; i--)
	{
		delete Nodes[i];
	}
	Nodes.Empty();
	RootNode = nullptr;
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory
}

void FNProcGenGraph::RegisterNode(FNProcGenGraphNode* Node)
{
	
	Nodes.Add(Node);
	Node->NodeID = Ticket++;
}
