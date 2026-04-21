// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Discriminator for a graph node's concrete subclass.
 */
enum class ENProcGenGraphNodeType
{
	Bone,
	Cell,
	Null
};

/**
 * Base class for every node in an FNProcGenGraph.
 *
 * Carries the shared world transform and node identifier; subclasses add cell/bone/null-specific
 * data. Nodes are heap-allocated, owned by the graph, and distinguishable at runtime via GetNodeType.
 */
class NEXUSPROCGEN_API FNProcGenGraphNode
{
	friend class FNProcGenGraph;
public:
	FNProcGenGraphNode(const FVector& Position, const FRotator& Rotation);

	virtual ~FNProcGenGraphNode() = default;

	/** @return The concrete node type. */
	virtual ENProcGenGraphNodeType GetNodeType() const = 0;

	/** @return The node's world-space position. */
	FVector GetWorldPosition() const { return WorldPosition; }

	/** @return The node's world-space rotation. */
	FRotator GetWorldRotation() const { return WorldRotation; }

	/** @return The node's stable identifier within its owning graph. */
	uint32 GetNodeID() const { return NodeID; }

protected:
	/** Subclass-only setter; the graph mutates transform during builder expansion. */
	void SetWorldPosition(const FVector& Position);

	/** Subclass-only setter; the graph mutates transform during builder expansion. */
	void SetWorldRotation(const FRotator& Rotation);

	/** Stable identifier assigned when the node is registered with the graph. */
	uint32 NodeID = 0;
private:

	/** World-space position. */
	FVector WorldPosition;

	/** World-space rotation. */
	FRotator WorldRotation;
};