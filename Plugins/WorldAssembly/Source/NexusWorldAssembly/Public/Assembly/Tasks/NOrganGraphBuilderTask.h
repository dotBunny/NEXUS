// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NPassContext.h"
#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Async/TaskGraphInterfaces.h"


class FNVirtualWorldContext;
class FNAssemblyTaskGraphContext;

/**
 * Task-graph job that builds the FNAssemblyGraph for a single organ using the Frontier model.
 *
 * Runs on a background worker; draws from the per-organ cell/bone input and expands outward
 * from each bone until no open junctions remain or the organ bounds/limits are hit.
 */
struct FNOrganGraphBuilderTask
{
	NEXUSWORLDASSEMBLY_API explicit FNOrganGraphBuilderTask(const TSharedPtr<FNVirtualOrganContext>& OrganContextPtr,
		const TSharedPtr<FNPassContext>& PassContextPtr, const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr,
		const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNWorldAssemblyGraphBuilderTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyNormalThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: performs the full organ build. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

	/**
	 * Force-place finisher-eligible cells that still fall short of their MinimumCount onto the graph's remaining
	 * open junctions, before the opportunistic CapBranchesWithFinishers pass can consume those slots.
	 *
	 * FinisherOnly cells are gated out of normal expansion and can only ever land at cap time, so without this a
	 * cell with MinimumCount > 0 has no guaranteed placement and CheckGraph legitimately fails its minimum. This
	 * is a no-op (no RNG draws, no graph changes) when no finisher-eligible cell has an unmet, enforceable minimum,
	 * keeping generation byte-identical for tissues that do not use the feature.
	 * @param Random Deterministic stream shared with the rest of the build.
	 */
	NEXUSWORLDASSEMBLY_API void EnsureFinisherMinimums(FNMersenneTwister& Random) const;

	/**
	 * @return true when CellNode's hull intersects any of the shared world-collision meshes (immutable after
	 *         FNProcessVirtualWorldTask) and exceeds the penetration threshold.
	 * @note Public so the perf tests can measure the real scan rather than a hand-rolled copy of its loop.
	 *       Scans every world-collision mesh, so its cost is O(world meshes) for every candidate placement.
	 */
	NEXUSWORLDASSEMBLY_API bool DoesWorldCollide(const FNAssemblyGraphCellNode* CellNode) const;

	/**
	 * @return Every existing cell whose world bounds intersect NewNode's.
	 * @note Public for the same reason as DoesWorldCollide. Walks every node in the organ graph (not just the
	 *       cells), so its cost grows with the organ as it is built.
	 */
	NEXUSWORLDASSEMBLY_API TArray<FNAssemblyGraphCellNode*> CheckNodeBounds(const FNAssemblyGraphCellNode* NewNode) const;

	/**
	 * @return true when CellNode's hull intersects any placed-cell hull visible to this organ and exceeds the
	 *         penetration threshold.
	 * @note Public for the same reason as DoesWorldCollide. Reads its extent and broadphase from the organ context
	 *       (NodeCollisionSnapshotCount / NodeCollisionBVH), so a caller that populates those can drive it directly.
	 */
	NEXUSWORLDASSEMBLY_API bool DoesExistingNodeWorldCollide(const FNAssemblyGraphCellNode* CellNode) const;

private:
	/** Per-organ input data and output graph reference. */
	TSharedRef<FNVirtualOrganContext> OrganContextPtr;

	/** Shared pass-level context. */
	TSharedRef<FNPassContext> PassContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
	N_ASSEMBLY_ANALYTICS_INDEX_LOCAL

	/** Virtual-world context supplying world-collision data for intersection tests. */
	TSharedRef<FNVirtualWorldContext> WorldContextPtr;

	/** Top-level task-graph context; used here to publish progress/status messages for UI. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	/** Seed the graph with the organ's bones and the root node. */
	void StartGraph(FNMersenneTwister& Random);

	/** @return Every existing cell whose hull intersects NewNode's hull. */
	TArray<FNAssemblyGraphCellNode*> CheckNodeHull(FNAssemblyGraphCellNode* NewNode) const;

	/** Expand the graph from SourceNode, returning the newly-created neighbours. */
	TArray<FNAssemblyGraphNode*> ProcessNode(FNMersenneTwister& Random, FNAssemblyGraphNode* SourceNode, bool bIsEndNode = false) const;

	/** Cell-node specialization of ProcessNode: picks candidates for each open junction. */
	TArray<FNAssemblyGraphNode*> ProcessCellNode(FNMersenneTwister& Random, FNAssemblyGraphCellNode* SourceCellNode, bool bIsEndNode = false) const;

	/**
	 * Resolve geometry for the chosen candidate, run the bounds/world/hull collision checks, and on success register
	 * and link it under SourceJunctionKey. Shared by ProcessCellNode (weighted random pick) and the finisher-minimum
	 * guarantee pass (forced pick) so both apply identical placement rules.
	 * @return The newly placed node, or nullptr when the candidate was rejected (out of bounds / colliding).
	 */
	FNAssemblyGraphCellNode* TryAttachCellToJunction(FNMersenneTwister& Random, FNAssemblyGraphCellNode* SourceCellNode,
		int32 SourceJunctionKey, const FNCellJunctionDetails* SourceJunctionValue, const FQuat& SourceJunctionWorldQuat,
		FNVirtualCellData* CellInputData, const TArray<int32>& ValidJunctionIndices) const;

	/**
	 * Attempt a single placement of the CellInputData[TargetCellIndex] candidate onto any compatible open junction
	 * in the current graph, in end-node (finisher) filter mode.
	 * @return true once the candidate was placed; false when no open junction can currently accept it.
	 */
	bool TryPlaceTargetCellOnce(FNMersenneTwister& Random, int32 TargetCellIndex) const;

	/** Cap open branch tips with Finisher/FinisherOnly cells when the tissue provides them. */
	void CapBranchesWithFinishers(FNMersenneTwister& Random) const;

	/** Remove a single cell node from the graph, cleaning up junctions, connections, null children, and unique tags. */
	void RemoveCellNode(FNAssemblyGraphCellNode* CellNode) const;

	/** Fixed-point pass that removes any NotFinisher-tagged leaf nodes from the graph. */
	void EnforceNotFinisherConstraint() const;
};
