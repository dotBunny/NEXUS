// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"

#include "Algo/Reverse.h"
#include "Containers/Deque.h"

/** @return The start cell: Root if it is a cell, otherwise the first cell linked downstream of it. */
static FNAssemblyGraphCellNode* ResolveStartCell(FNAssemblyGraphNode* Root)
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
static TArray<FNAssemblyGraphNode*> BreadthFirstPathTo(FNAssemblyGraphNode* Source, TPredicate&& IsTarget)
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

/**
 * Multi-source shortest-hop sweep from Seeds over the undirected graph (downstream ∪ upstream), assigning each
 * reachable cell node the number of cells between it and the nearest seed, via Assign. Seeds score 0.
 *
 * Only entering a *cell* costs a hop: bones and null terminators are stepped through for free, so the score reads
 * as "cells away" rather than "nodes away". That makes the edge weights 0 and 1 rather than uniformly 1, which is
 * why this is a deque sweep (zero-cost steps to the front, one-cost to the back) rather than a plain queue BFS.
 * As the graph stands the two would agree — a bone owns one socket and a null node caps one junction, so neither
 * can ever join two cells and shortcut the count — but nothing in the traversal has to rely on that.
 *
 * Scores saturate at FNCellAssemblyData::UnreachableScore: a cell that would score there is neither assigned nor
 * expanded from, so it stays indistinguishable from one no seed reaches. That bound is also what keeps the sweep
 * from being able to run away on a pathological graph.
 */
template <typename TAssign>
static void ScoreCellHopsFromSeeds(const TArray<FNAssemblyGraphCellNode*>& Seeds, TAssign&& Assign)
{
	if (Seeds.IsEmpty()) return;

	// Best score known for each node so far; also the visited set. Non-cell nodes are tracked too, since a cheaper
	// route can reach one after it has already been stepped through.
	TMap<FNAssemblyGraphNode*, uint8> Scores;
	Scores.Reserve(Seeds.Num() * 8);

	TDeque<FNAssemblyGraphNode*> Frontier;
	for (FNAssemblyGraphCellNode* Seed : Seeds)
	{
		if (Scores.Contains(Seed)) continue;
		Scores.Add(Seed, 0);
		Assign(Seed, 0);
		Frontier.EmplaceLast(Seed);
	}

	while (!Frontier.IsEmpty())
	{
		FNAssemblyGraphNode* Current = Frontier.First();
		Frontier.PopFirst();

		const uint8 CurrentScore = Scores[Current];

		// Treat the graph as undirected: a connector edge and a junction mating are both walkable either way.
		for (int32 Direction = 0; Direction < 2; Direction++)
		{
			const TArray<FNAssemblyGraphNode*>& Neighbours =
				Direction == 0 ? Current->GetDownstreamNodes() : Current->GetUpstreamNodes();
			for (FNAssemblyGraphNode* Neighbour : Neighbours)
			{
				const bool bIsCell = Neighbour->GetNodeType() == ENAssemblyGraphNodeType::Cell;
				const int32 Candidate = static_cast<int32>(CurrentScore) + (bIsCell ? 1 : 0);

				// At the saturation point the score stops carrying information, so stop rather than record it.
				if (Candidate >= FNCellAssemblyData::UnreachableScore) continue;

				const uint8* Known = Scores.Find(Neighbour);
				if (Known != nullptr && *Known <= Candidate) continue;

				const uint8 NewScore = static_cast<uint8>(Candidate);
				Scores.Add(Neighbour, NewScore);
				if (bIsCell)
				{
					Assign(static_cast<FNAssemblyGraphCellNode*>(Neighbour), NewScore);
					Frontier.EmplaceLast(Neighbour);
				}
				else
				{
					// Free to step through, so it belongs ahead of everything a hop further out.
					Frontier.EmplaceFirst(Neighbour);
				}
			}
		}
	}
}

/** Apply Setter to every cell node in OnPath (non-cell nodes carry no hot path flag). */
template <typename TSetter>
static void FlagCellsOnPath(const TSet<FNAssemblyGraphNode*>& OnPath, TSetter&& Setter)
{
	for (FNAssemblyGraphNode* Node : OnPath)
	{
		if (Node->GetNodeType() == ENAssemblyGraphNodeType::Cell)
		{
			Setter(static_cast<FNAssemblyGraphCellNode*>(Node));
		}
	}
}

FNAssemblyGraph::FNAssemblyGraph(FNAssemblyGraphNode* RootNodePtr, const FVector& Origin, const FBoxSphereBounds& Bounds, const bool bUnbounded)
: bUnbounded(bUnbounded), Bounds(Bounds), Origin(Origin), RootNode(RootNodePtr)
{
	// Root is added directly rather than through RegisterNode, so seed the cached cell count and position sum to match.
	if (RootNodePtr != nullptr && RootNodePtr->GetNodeType() == ENAssemblyGraphNodeType::Cell)
	{
		FNAssemblyGraphCellNode* RootCellNode = static_cast<FNAssemblyGraphCellNode*>(RootNodePtr);
		CellNodeCount++;
		CellPositionSum += RootCellNode->GetWorldBoundsCenter();
		CellNodes.Add(RootCellNode);
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
		FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
		CellNodeCount++;
		CellPositionSum += CellNode->GetWorldBoundsCenter();
		// Appended, not indexed: the new entry joins the unindexed tail and is picked up by the next rebuild.
		CellNodes.Add(CellNode);
		if (FNVirtualCellData* InputData = CellNode->GetInputDataPtr())
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
		FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
		CellNodeCount--;
		CellPositionSum -= CellNode->GetWorldBoundsCenter();
		CellNodes.Remove(CellNode);
		// Removal shifts every index past this one, so the tree no longer describes CellNodes. Drop it rather than
		// patch it; removals are rare next to insertions and a full rebuild on next query is simpler to be sure of.
		bCellNodeIndexDirty = true;
		if (FNVirtualCellData* InputData = CellNode->GetInputDataPtr())
		{
			InputData->UsedCount--;
		}
	}

	Nodes.Remove(Node);
}

void FNAssemblyGraph::EnsureCellNodeIndex() const
{
	if (bCellNodeIndexDirty)
	{
		bCellNodeIndexDirty = false;
		IndexedCellNodeCount = 0;
		CellNodeBVH = FNBoundsBVH();
	}

	// Everything past IndexedCellNodeCount is scanned linearly by the query. Rebuild once that tail is long enough
	// to be worth the O(N log N); below the threshold the scan is cheaper than the rebuild would be.
	if (CellNodes.Num() - IndexedCellNodeCount <= CellNodeIndexTailThreshold)
	{
		return;
	}

	TArray<FBox> CellBounds;
	CellBounds.Reserve(CellNodes.Num());
	for (const FNAssemblyGraphCellNode* CellNode : CellNodes)
	{
		CellBounds.Add(CellNode->GetWorldBounds());
	}
	CellNodeBVH = FNBoundsBVH(CellBounds);
	IndexedCellNodeCount = CellNodes.Num();
}

void FNAssemblyGraph::QueryCellNodesByBounds(const FBox& QueryBounds, TArray<FNAssemblyGraphCellNode*>& OutNodes) const
{
	OutNodes.Reset();

	EnsureCellNodeIndex();

	if (IndexedCellNodeCount > 0)
	{
		TArray<int32, TInlineAllocator<32>> Overlaps;
		CellNodeBVH.QueryOverlaps(QueryBounds, Overlaps);
		for (const int32 Index : Overlaps)
		{
			OutNodes.Add(CellNodes[Index]);
		}
	}

	// Nodes registered since the last rebuild are not in the tree yet; the threshold keeps this short.
	for (int32 i = IndexedCellNodeCount; i < CellNodes.Num(); i++)
	{
		if (CellNodes[i]->CheckBoundsIntersects(QueryBounds))
		{
			OutNodes.Add(CellNodes[i]);
		}
	}
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

void FNAssemblyGraph::ScoreCellProximity(TArrayView<const TUniquePtr<FNAssemblyGraph>> Graphs)
{
	// Collected in one pass over every graph, so the sweeps that follow start from the operation's whole seed set
	// rather than one graph's share of it.
	TArray<FNAssemblyGraphCellNode*> HotPathShortestSeeds;
	TArray<FNAssemblyGraphCellNode*> HotPathSequentialSeeds;
	TArray<FNAssemblyGraphCellNode*> ImportantSeeds;

	for (const TUniquePtr<FNAssemblyGraph>& Graph : Graphs)
	{
		if (!Graph.IsValid()) continue;
		for (FNAssemblyGraphNode* Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
			if (CellNode->IsHotPathShortest())
			{
				HotPathShortestSeeds.Add(CellNode);
			}
			if (CellNode->IsHotPathSequential())
			{
				HotPathSequentialSeeds.Add(CellNode);
			}
			// Seeded from cells only. The flag is declared on the node base so a bone can carry the tag, but an
			// importance score counts cells away from an important *cell*, and a bone places no content to be near.
			if (CellNode->IsImportantFlagged())
			{
				ImportantSeeds.Add(CellNode);
			}
		}
	}

	// Three independent sweeps rather than one: the variants disagree wherever the sequential chain and the
	// shortest spokes route differently, and a cell can be adjacent to one and far from the other.
	ScoreCellHopsFromSeeds(HotPathShortestSeeds,
		[](FNAssemblyGraphCellNode* Cell, const uint8 Score) { Cell->SetHotPathShortestScore(Score); });
	ScoreCellHopsFromSeeds(HotPathSequentialSeeds,
		[](FNAssemblyGraphCellNode* Cell, const uint8 Score) { Cell->SetHotPathSequentialScore(Score); });
	ScoreCellHopsFromSeeds(ImportantSeeds,
		[](FNAssemblyGraphCellNode* Cell, const uint8 Score) { Cell->SetImportanceScore(Score); });
}


