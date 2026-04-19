// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenGraphNode.h"

class NEXUSPROCGEN_API FNProcGenGraph
{
public:
	explicit FNProcGenGraph(FNProcGenGraphNode* RootNodePtr, const FVector& Origin, const FBoxSphereBounds& Bounds, bool bUnbounded = false);
	~FNProcGenGraph();

	FNProcGenGraphNode* GetLastNode() { return Nodes.Last(); }

	const TArray<FNProcGenGraphNode*>& GetNodes() const { return Nodes; }
	TArray<FNProcGenGraphNode*>& GetMutableNodes() { return Nodes; }
	
	void RegisterNode(FNProcGenGraphNode* Node);
	TArray<FNProcGenGraphNode*> GetNodesWithOpenJunctions();
	
	int GetNodeCount() const
	{
		return Nodes.Num();
	}
	
	bool IsUnbounded() const { return bUnbounded; }
	
private:
	bool bUnbounded = false;
	FBoxSphereBounds Bounds;
	FVector Origin;
	
	uint32 Ticket = 0;
	FNProcGenGraphNode* RootNode = nullptr;
	TArray<FNProcGenGraphNode*> Nodes;
};
