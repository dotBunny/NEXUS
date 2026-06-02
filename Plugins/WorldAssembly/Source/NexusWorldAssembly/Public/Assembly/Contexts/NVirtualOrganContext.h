// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"

struct FNWorldOrganData;
class UNOrganComponent;

/**
 * Query bundle passed into FNVirtualOrganContext::FilterCellInputData to narrow down candidate cells.
 */
struct FNCellInputDataFilter
{
	/**
	 * Hop-count from the start node of the cell the filter is stepping away from.
	 * Compared against each candidate's MinimumNodeDepth to gate cells that are only allowed
	 * to appear at or beyond a certain distance from the bone. 0 means "at the start".
	 */
	int32 NodeDepth = 0;

	/** When true, only cells flagged bCanBeStartNode are eligible. */
	bool bIsStartNode = false;

	/** When true, only cells flagged bCanBeEndNode are eligible. */
	bool bIsEndNode = false;

	/** Cell the filter is stepping away from; used for distance/uniqueness checks. */
	FNVirtualCellData* SourceCellInputData = nullptr;

	/** Graph node the filter is stepping away from; used for spatial checks. */
	FNAssemblyGraphCellNode* SourceCellNode = nullptr;

	/** Socket size the candidate cell must provide at least one junction of. */
	FIntVector2 SocketSize = FIntVector2(0, 0);

	/** Orientation applied to the candidate's junction to match the source. */
	FQuat SourceQuat = FQuat();
};

/**
 * Per-organ context consumed by FNOrganGraphBuilderTask.
 *
 * Holds the input cell/bone pools, spatial constraints, and the output graph. Re-entrant across
 * retries: ResetForRetry drops the in-progress graph while preserving seed/name so analytics
 * remain aggregated.
 */
class FNVirtualOrganContext
{
	friend struct FNOrganGeneratorBuildGraphTask;

public:
	
	/** Allowed penetration, in world units, between adjacent cell hulls before they are treated as overlapping. */
	float CellHullPenetration = 10.f;

	/** Allowed penetration, in world units, between a placed cell hull and existing world collision before it is rejected. */
	float WorldHullPenetration = 1.f;

	/** World-space size of a single voxel, cached from UNWorldAssemblySettings so placed cells re-voxelize without re-reading settings. */
	FVector VoxelSize = FVector(100.f, 100.f, 100.f);

	/** Lower bound on cell count; -1 disables the check. */
	int32 MinimumCellCount = -1;

	/** Upper bound on cell count; -1 disables the check. */
	int32 MaximumCellCount = -1;

	/** Number of retries this organ is allowed before giving up. */
	int32 MaximumRetryCount = 0;
	
	/** Maximum number of failed start attempts before the build gives up on this organ. */
	int32 BadStartLimit = 1000;

	/** When true, the graph may extend past Bounds. */
	bool bUnbounded = false;

	/** Spatial bounds the graph must stay within unless bUnbounded. */
	FBoxSphereBounds Bounds;

	/** World origin of the organ used as the graph root. */
	FVector Origin;

	/** Tag groups accumulated from cells already placed in the graph. */
	FNTissueTagGroups PlacedTagGroups;

	/** Context tags present at the start of organ build, used to ensure we do not lose context as cells get removed. */
	FGameplayTagContainer BaseContextTags;
	
	/** Context tags currently active, updated as cells are placed into the graph. */
	FGameplayTagContainer ContextTags;

	/** Bones the builder will anchor the graph on. */
	TArray<FNVirtualBoneData> BoneInputData;

	/** Candidate cells the builder may pull from. */
	TArray<FNVirtualCellData> CellInputData;
	
	/** Cached classification summary of CellInputData (starter/finisher availability and tag groups). */
	FNVirtualCellDataSummary CellInputDataSummary;

	/** Output graph, owned by this context until handed off to the task-graph context. */
	TUniquePtr<FNAssemblyGraph> CellGraph = nullptr;

	FNVirtualOrganContext(const FNWorldOrganData* WorldOrganContext, uint64 TaskSeed, FString TaskName);
	~FNVirtualOrganContext();

	/** @return Deterministic seed used to drive this organ's random stream. */
	uint64 GetSeed() const { return Seed; };

	/** @return Human-readable task name for logs/debug. */
	const FString& GetName() { return Name; };

	/** @return true once the builder has produced a valid graph (cell count within bounds, etc). */
	bool IsSuccessful() const { return bSuccessful; };

	/** @return true if the context is set up well enough to attempt a build. */
	bool IsValid() const { return bIsValid; };

	/** @return true if the current graph satisfies all required invariants (debug helper). */
	bool CheckGraph();

	/**
	 * Filter CellInputData into a weighted candidate list for selection.
	 * @param Filter Query constraints (socket size, start/end requirements, source node).
	 * @param CellIndices [out] Indices into CellInputData weighted by their configured weighting.
	 * @param JunctionIndices [out] For each cell index, the subset of its junction keys that match the filter.
	 */
	void FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices);


	/**
	 * Resolve which bad-neighbor groups the source cell belongs to. Returns empty when there is no source
	 * node (e.g. the start-node pre-filter) or when no bad-neighbor groups are configured, which disables
	 * the per-candidate bad-neighbor check.
	 * @param GroupTags Tag-group registry holding the configured bad-neighbor groups.
	 * @param SourceCellNode Node the filter is stepping away from, or nullptr for the start pre-filter.
	 * @return The subset of the source's assembly tags that name a bad-neighbor group.
	 */
	static NEXUSWORLDASSEMBLY_API FGameplayTagContainer ResolveSourceBadNeighborTags(const FNTissueTagGroups& GroupTags, FNAssemblyGraphCellNode* SourceCellNode);

	/**
	 * @param SourceBadNeighborTags Bad-neighbor groups the source belongs to (from ResolveSourceBadNeighborTags).
	 * @param Candidate Cell being considered for placement beside the source.
	 * @return true if Candidate shares a bad-neighbor group with the source and must not be placed beside it.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsBadNeighbor(const FGameplayTagContainer& SourceBadNeighborTags, const FNVirtualCellData& Candidate);

	/**
	 * @param bIsStartNode true when filtering candidates for the graph's start node.
	 * @param Summary Pool summary carrying whether any starter / starter-only cells exist.
	 * @param CandidateTags Assembly tags of the candidate cell.
	 * @return true if the candidate is excluded by the starter tag rules.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByStarterTags(bool bIsStartNode, const FNVirtualCellDataSummary& Summary, const FGameplayTagContainer& CandidateTags);

	/**
	 * @param bIsEndNode true when filtering candidates for the graph's end node.
	 * @param Summary Pool summary carrying whether any finisher / finisher-only cells exist.
	 * @param CandidateTags Assembly tags of the candidate cell.
	 * @return true if the candidate is excluded by the finisher tag rules.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByFinisherTags(bool bIsEndNode, const FNVirtualCellDataSummary& Summary, const FGameplayTagContainer& CandidateTags);

	/**
	 * Gate a candidate by minimum graph depth. SourceNodeDepth is the depth of the node the filter is
	 * stepping away from; because graph depth is rooted at the bone (start cell = depth 1) and the candidate
	 * lands one hop deeper, the two offsets cancel and this resolves to "hops from the start cell".
	 * @param MinimumNodeDepth The candidate's configured minimum depth; 0 disables the gate.
	 * @param SourceNodeDepth Depth of the source node the filter is stepping away from.
	 * @return true if the candidate is too shallow and must be gated out.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByMinimumNodeDepth(int32 MinimumNodeDepth, int32 SourceNodeDepth);

	/**
	 * Drop the current graph and bump the retry counter.
	 * @return true if another retry is allowed; false once MaximumRetryCount is exhausted.
	 */
	bool ResetForRetry();

	/** Run the post-build invariant checks; flips bSuccessful on success. */
	bool ValidateGraph();

private:
	/** Number of retries consumed so far. */
	int32 RetryCount = 0;

	/** True once construction inputs have been validated. */
	bool bIsValid = false;

	/** True once ValidateGraph accepts the produced graph. */
	bool bSuccessful = false;

	/** Random-stream seed for this organ's build. */
	uint64 Seed;

	/** Human-readable task name used in logs. */
	FString Name;
};
