// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"

FNAssemblyGraph::FNAssemblyGraph(FNAssemblyGraphNode* RootNodePtr, const FVector& Origin, const FBoxSphereBounds& Bounds, const bool bUnbounded)
: bUnbounded(bUnbounded), Bounds(Bounds), Origin(Origin), RootNode(RootNodePtr)
{
	Nodes.Add(RootNodePtr);
}

FNAssemblyGraph::~FNAssemblyGraph()
{
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory
	const int32 NodeCount = Nodes.Num();
	if (NodeCount > 0)
	{
		for (int32 i = NodeCount - 1; i >= 0; i--)
		{
			delete Nodes[i];
		}
		Nodes.Empty();
	}
	RootNode = nullptr;
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory
}

void FNAssemblyGraph::RegisterNode(FNAssemblyGraphNode* Node)
{
	// A cell instance begins when its node joins the graph. Counting here (rather than per-junction in
	// LinkJunction) keeps UsedCount aligned with placed instances, not junction connections.
	if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
	{
		if (FNVirtualCellData* InputData = static_cast<FNAssemblyGraphCellNode*>(Node)->GetInputDataPtr())
		{
			InputData->UsedCount++;
		}
	}

	Nodes.Add(Node);
	Node->NodeIdentifier = Ticket++;
}

void FNAssemblyGraph::UnregisterNode(FNAssemblyGraphNode* Node)
{
	// Mirror of RegisterNode: the instance ends when the node leaves the graph.
	if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
	{
		if (FNVirtualCellData* InputData = static_cast<FNAssemblyGraphCellNode*>(Node)->GetInputDataPtr())
		{
			InputData->UsedCount--;
		}
	}

	Nodes.Remove(Node);
}

TArray<FNAssemblyGraphNode*> FNAssemblyGraph::GetNodesWithOpenJunctions()
{
	TArray<FNAssemblyGraphNode*> ReturnNodes;
	for (FNAssemblyGraphNode* Node : Nodes)
	{
		if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
		{
			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
			if (CellNode->HasOpenJunctions())
			{
				ReturnNodes.Add(CellNode);
			}
		}
	}
	return MoveTemp(ReturnNodes);
}

void FNAssemblyGraph::CleanupBuilderReferences()
{
	if (Nodes.IsEmpty()) return;
	
	for (FNAssemblyGraphNode* Node : Nodes)
	{
		if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
		{
			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
			CellNode->CleanupBuilderReferences();
		}
	}
}


