// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Discriminator for a graph node's concrete subclass.
 */
enum class ENAssemblyGraphNodeType
{
	Bone,
	Cell,
	Null
};

/**
 * Base class for every node in an FNAssemblyGraph.
 *
 * Carries the shared world transform and node identifier; subclasses add cell/bone/null-specific
 * data. Nodes are heap-allocated, owned by the graph, and distinguishable at runtime via GetNodeType.
 */
class NEXUSWORLDASSEMBLY_API FNAssemblyGraphNode
{
	friend class FNAssemblyGraph;
public:
	FNAssemblyGraphNode(const FVector& Position, const FRotator& Rotation);

	virtual ~FNAssemblyGraphNode() = default;

	/** @return The concrete node type. */
	virtual ENAssemblyGraphNodeType GetNodeType() const = 0;

	/** @return The node's world-space position. */
	FVector GetWorldPosition() const { return WorldPosition; }

	/** @return The node's world-space rotation. */
	FRotator GetWorldRotation() const { return WorldRotation; }

	/** @return The node's stable identifier within its owning graph. */
	uint32 GetNodeID() const { return NodeID; }
	
	int32 GetNodeDepth() const { return NodeDepth; }

	/**
	 * Wire Upstream -> Downstream and relax Downstream's NodeDepth (and its subtree) so depth tracks
	 * the shortest hop-count back to the start node. Safe to call in any order: if Upstream is not yet
	 * reachable (NodeDepth still MAX_int32) the depth update is skipped and will happen once Upstream
	 * itself gets a depth.
	 */
	void Connect(FNAssemblyGraphNode* DownstreamNode)
	{
		DownstreamNodes.AddUnique(DownstreamNode);
		DownstreamNode->UpstreamNodes.AddUnique(this);

		if (NodeDepth == MAX_int32) return;

		const int32 Candidate = NodeDepth + 1;
		if (Candidate >= DownstreamNode->NodeDepth) return;

		DownstreamNode->NodeDepth = Candidate;
		PropagateDepth(DownstreamNode);
	}

protected:
	/** Subclass-only setter; the graph mutates transform during builder expansion. */
	void SetWorldPosition(const FVector& Position);

	/** Subclass-only setter; the graph mutates transform during builder expansion. */
	void SetWorldRotation(const FRotator& Rotation);

	/** Stable identifier assigned when the node is registered with the graph. */
	uint32 NodeID = 0;

	/** Hop-count from the start node along the shortest upstream path; MAX_int32 until reachable. */
	int32 NodeDepth = MAX_int32;

	TArray<FNAssemblyGraphNode*> UpstreamNodes;
	TArray<FNAssemblyGraphNode*> DownstreamNodes;

private:
	static void PropagateDepth(FNAssemblyGraphNode* Root)
	{
		TQueue<FNAssemblyGraphNode*> Frontier;
		Frontier.Enqueue(Root);
		FNAssemblyGraphNode* Current = nullptr;
		while (Frontier.Dequeue(Current))
		{
			const int32 ChildDepth = Current->NodeDepth + 1;
			for (FNAssemblyGraphNode* Child : Current->DownstreamNodes)
			{
				if (ChildDepth < Child->NodeDepth)
				{
					Child->NodeDepth = ChildDepth;
					Frontier.Enqueue(Child);
				}
			}
		}
	}

	/** World-space position. */
	FVector WorldPosition;

	/** World-space rotation. */
	FRotator WorldRotation;
};