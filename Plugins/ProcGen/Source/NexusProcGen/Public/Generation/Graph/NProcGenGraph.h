// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenGraphNode.h"

/**
 * In-memory graph describing a single organ's generated layout.
 *
 * Owns the set of FNProcGenGraphNode instances (cells, bones, nulls) produced by the graph
 * builder and the spatial extents they must stay within. Nodes are registered as they are
 * expanded; CleanupBuilderReferences is called once the graph is handed to the next stage.
 */
class NEXUSPROCGEN_API FNProcGenGraph
{
public:
	explicit FNProcGenGraph(FNProcGenGraphNode* RootNodePtr, const FVector& Origin, const FBoxSphereBounds& Bounds, bool bUnbounded = false);
	~FNProcGenGraph();

	/** @return The most recently registered node. */
	FNProcGenGraphNode* GetLastNode() { return Nodes.Last(); }

	/** @return All nodes currently in the graph, in registration order. */
	const TArray<FNProcGenGraphNode*>& GetNodes() const { return Nodes; }

	/** @return Mutable access to all nodes, for builder/collector mutation. */
	TArray<FNProcGenGraphNode*>& GetMutableNodes() { return Nodes; }

	/** Take ownership of Node and add it to the graph. */
	void RegisterNode(FNProcGenGraphNode* Node);

	/** @return Every node that still has at least one unconnected junction. */
	TArray<FNProcGenGraphNode*> GetNodesWithOpenJunctions();

	/** @return Total node count. */
	int32 GetNodeCount() const
	{
		return Nodes.Num();
	}

	/** @return true if the graph was allowed to extend outside its organ bounds. */
	bool IsUnbounded() const { return bUnbounded; }

	/** Drop builder-only scratch state from each node; call before handing the graph to the spawn stage. */
	void CleanupBuilderReferences();

private:
	/** Whether the graph is permitted to extend past its organ bounds. */
	bool bUnbounded = false;

	/** Spatial bounds the graph must stay inside unless bUnbounded. */
	FBoxSphereBounds Bounds;

	/** World origin of the graph, used when positioning child nodes. */
	FVector Origin;

	/** Ticket of the operation that built this graph. */
	uint32 Ticket = 0;

	/** Starting node the builder expanded from. */
	FNProcGenGraphNode* RootNode = nullptr;

	/** All owned nodes, in registration order. */
	TArray<FNProcGenGraphNode*> Nodes;
};
