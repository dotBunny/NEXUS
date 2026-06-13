// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"

#include "Algo/Reverse.h"

namespace
{
	/** @return The start cell: Root if it is a cell, otherwise the first cell linked downstream of it. */
	FNAssemblyGraphCellNode* ResolveStartCell(FNAssemblyGraphNode* Root)
	{
		if (Root == nullptr) return nullptr;
		if (Root->GetNodeType() == ENAssemblyGraphNodeType::Cell)
		{
			return static_cast<FNAssemblyGraphCellNode*>(Root);
		}
		for (FNAssemblyGraphNode* Downstream : Root->GetDownstreamNodes())
		{
			if (Downstream->GetNodeType() == ENAssemblyGraphNodeType::Cell)
			{
				return static_cast<FNAssemblyGraphCellNode*>(Downstream);
			}
		}
		return nullptr;
	}

	/**
	 * Unweighted BFS from Source over the undirected graph (downstream ∪ upstream), stopping at the first
	 * node satisfying IsTarget. Because every edge has weight 1, the first target reached is a shortest one.
	 * @return The node path from Source to that target inclusive, or empty if no target is reachable.
	 */
	template <typename TPredicate>
	TArray<FNAssemblyGraphNode*> BreadthFirstPathTo(FNAssemblyGraphNode* Source, TPredicate&& IsTarget)
	{
		TArray<FNAssemblyGraphNode*> Path;
		if (Source == nullptr) return Path;

		if (IsTarget(Source))
		{
			Path.Add(Source);
			return Path;
		}

		// node -> predecessor along the BFS tree; also doubles as the visited set.
		TMap<FNAssemblyGraphNode*, FNAssemblyGraphNode*> CameFrom;
		CameFrom.Add(Source, nullptr);

		TQueue<FNAssemblyGraphNode*> Frontier;
		Frontier.Enqueue(Source);

		FNAssemblyGraphNode* Found = nullptr;
		FNAssemblyGraphNode* Current = nullptr;
		while (Found == nullptr && Frontier.Dequeue(Current))
		{
			// Treat the graph as undirected: scan both directions.
			for (int32 Direction = 0; Direction < 2 && Found == nullptr; Direction++)
			{
				const TArray<FNAssemblyGraphNode*>& Neighbours =
					Direction == 0 ? Current->GetDownstreamNodes() : Current->GetUpstreamNodes();
				for (FNAssemblyGraphNode* Neighbour : Neighbours)
				{
					if (CameFrom.Contains(Neighbour)) continue;
					CameFrom.Add(Neighbour, Current);
					if (IsTarget(Neighbour))
					{
						Found = Neighbour;
						break;
					}
					Frontier.Enqueue(Neighbour);
				}
			}
		}

		if (Found == nullptr) return Path;

		// Walk predecessors back to Source, then flip to Source -> Found order.
		for (FNAssemblyGraphNode* Node = Found; Node != nullptr; Node = CameFrom[Node])
		{
			Path.Add(Node);
		}
		Algo::Reverse(Path);
		return Path;
	}

	/** Apply Setter to every cell node in OnPath (non-cell nodes carry no hot path flag). */
	template <typename TSetter>
	void FlagCellsOnPath(const TSet<FNAssemblyGraphNode*>& OnPath, TSetter&& Setter)
	{
		for (FNAssemblyGraphNode* Node : OnPath)
		{
			if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
			{
				Setter(static_cast<FNAssemblyGraphCellNode*>(Node));
			}
		}
	}
}

FNAssemblyGraph::FNAssemblyGraph(FNAssemblyGraphNode* RootNodePtr, const FVector& Origin, const FBoxSphereBounds& Bounds, const bool bUnbounded)
: bUnbounded(bUnbounded), Bounds(Bounds), Origin(Origin), RootNode(RootNodePtr)
{
	// Root is added directly rather than through RegisterNode, so seed the cached cell count to match.
	if (RootNodePtr != nullptr && RootNodePtr->GetNodeType() == ENAssemblyGraphNodeType::Cell)
	{
		CellNodeCount++;
	}
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
		CellNodeCount++;
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
		CellNodeCount--;
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
	return ReturnNodes;
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

void FNAssemblyGraph::FlagHotPath()
{
	FNAssemblyGraphCellNode* StartCell = ResolveStartCell(RootNode);
	if (StartCell == nullptr) return;

	// Goals are the Hotpath-flagged cells; extra bones are any bone nodes other than the root.
	TArray<FNAssemblyGraphCellNode*> Goals;
	TArray<FNAssemblyGraphNode*> ExtraBones;
	for (FNAssemblyGraphNode* Node : Nodes)
	{
		switch (Node->GetNodeType())
		{
		case ENAssemblyGraphNodeType::Cell:
			if (Node->IsHotPathFlagged())
			{
				Goals.Add(static_cast<FNAssemblyGraphCellNode*>(Node));
			}
			break;
		case ENAssemblyGraphNodeType::Bone:
			if (Node != RootNode)
			{
				ExtraBones.Add(Node);
			}
			break;
		default:
			break;
		}
	}

	// With no goals there is no hot path; leave every flag at its default false.
	if (Goals.Num() == 0) return;

	// Branch from the nearest already-on-path node out to each additional bone. Shared by both variants.
	auto BranchToBones = [&ExtraBones](TSet<FNAssemblyGraphNode*>& OnPath)
	{
		for (FNAssemblyGraphNode* Bone : ExtraBones)
		{
			TArray<FNAssemblyGraphNode*> Path = BreadthFirstPathTo(Bone,
				[&OnPath](FNAssemblyGraphNode* Node) { return OnPath.Contains(Node); });
			OnPath.Append(Path);
		}
	};

	// Shortest variant: union of the independent shortest path from the start cell to each goal.
	{
		TSet<FNAssemblyGraphNode*> OnPath;
		for (FNAssemblyGraphCellNode* Goal : Goals)
		{
			TArray<FNAssemblyGraphNode*> Path = BreadthFirstPathTo(StartCell,
				[Goal](FNAssemblyGraphNode* Node) { return Node == Goal; });
			OnPath.Append(Path);
		}
		BranchToBones(OnPath);
		FlagCellsOnPath(OnPath, [](FNAssemblyGraphCellNode* Cell) { Cell->SetHotPathShortest(true); });
	}

	// Sequential variant: thread start -> nearest goal -> next-nearest -> ... unioning each leg.
	{
		TSet<FNAssemblyGraphNode*> OnPath;
		OnPath.Add(StartCell);

		TSet<FNAssemblyGraphNode*> RemainingGoals;
		for (FNAssemblyGraphCellNode* Goal : Goals)
		{
			RemainingGoals.Add(Goal);
		}
		RemainingGoals.Remove(StartCell); // No-op unless the start cell is itself a goal.

		FNAssemblyGraphNode* Current = StartCell;
		while (RemainingGoals.Num() > 0)
		{
			TArray<FNAssemblyGraphNode*> Path = BreadthFirstPathTo(Current,
				[&RemainingGoals](FNAssemblyGraphNode* Node) { return RemainingGoals.Contains(Node); });
			if (Path.Num() == 0) break; // Remaining goals are unreachable from the current node.

			Current = Path.Last();
			OnPath.Append(Path);
			RemainingGoals.Remove(Current);
		}
		BranchToBones(OnPath);
		FlagCellsOnPath(OnPath, [](FNAssemblyGraphCellNode* Cell) { Cell->SetHotPathSequential(true); });
	}
}


