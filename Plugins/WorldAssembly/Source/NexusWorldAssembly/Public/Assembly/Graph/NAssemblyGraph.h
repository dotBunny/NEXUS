// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NAssemblyGraphNode.h"

/**
 * In-memory graph describing a single organ's generated layout.
 *
 * Owns the set of FNAssemblyGraphNode instances (cells, bones, nulls) produced by the graph
 * builder and the spatial extents they must stay within. Nodes are registered as they are
 * expanded; CleanupBuilderReferences is called once the graph is handed to the next stage.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyGraph
{
public:
	explicit FNAssemblyGraph(FNAssemblyGraphNode* RootNodePtr, const FVector& Origin, const FBoxSphereBounds& Bounds, bool bUnbounded = false);
	~FNAssemblyGraph();

	/** @return The most recently registered node. */
	FNAssemblyGraphNode* GetLastNode() { return Nodes.Last(); }

	/** @return The starting node the graph was expanded from (usually the root bone). */
	const FNAssemblyGraphNode* GetRootNode() const { return RootNode; }

	/**
	 * Compute this graph's hot path and flag the cell nodes that lie on it.
	 *
	 * Starting from the start cell (the root, or the cell linked to the root bone), threads a path
	 * through every Hotpath-flagged cell and branches out to any additional bones. Two variants are
	 * resolved per cell: the shortest-path spokes from the start, and the nearest-first visiting chain.
	 * Both use unweighted (hop-count) BFS over the undirected node connectivity. Must run before
	 * FNAssemblyGraphCellNode::GenerateLinkDetails so the per-junction flags can read neighbour state.
	 */
	void FlagHotPath();

	/** @return All nodes currently in the graph, in registration order. */
	const TArray<FNAssemblyGraphNode*>& GetNodes() const { return Nodes; }

	/** Take ownership of Node and add it to the graph. */
	void RegisterNode(FNAssemblyGraphNode* Node);

	/** Remove Node from the graph's ownership array without deleting it; caller takes ownership. */
	void UnregisterNode(FNAssemblyGraphNode* Node);

	/** @return Every node that still has at least one unconnected junction. */
	TArray<FNAssemblyGraphNode*> GetNodesWithOpenJunctions();

	/** @return Total node count. */
	int32 GetNodeCount() const
	{
		return Nodes.Num();
	}

	/** @return Number of Cell-type nodes only (excludes Bone and Null nodes). */
	int32 GetCellNodeCount() const
	{
		return CellNodeCount;
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
	FNAssemblyGraphNode* RootNode = nullptr;

	/** All owned nodes, in registration order. */
	TArray<FNAssemblyGraphNode*> Nodes;

	/** Cached count of Cell-type nodes, kept in sync by Register/UnregisterNode to avoid O(N) recounts. */
	int32 CellNodeCount = 0;
};
