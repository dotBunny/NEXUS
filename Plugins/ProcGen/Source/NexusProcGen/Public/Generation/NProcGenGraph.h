// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenGraphNode.h"

class NEXUSPROCGEN_API FNProcGenGraph
{
public:
	explicit FNProcGenGraph(FNProcGenGraphNode* RootNodePtr);
	~FNProcGenGraph();

	FNProcGenGraphNode* GetLastNode() { return Nodes.Last(); }

	const TArray<FNProcGenGraphNode*>& GetNodes() const { return Nodes; }
	TArray<FNProcGenGraphNode*>& GetMutableNodes() { return Nodes; }
	
	void RegisterNode(FNProcGenGraphNode* Node);
	
	int GetNodeCount() const
	{
		return Nodes.Num();
	}
private:
	
	FNProcGenGraphNode* RootNode = nullptr;
	TArray<FNProcGenGraphNode*> Nodes;
};
