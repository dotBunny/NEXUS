// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Data/NBoneInputData.h"
#include "Generation/Graph/NProcGenGraph.h"
#include "Generation/Data/NCellInputData.h"
#include "Generation/Tasks/NOrganGraphBuilderAnalytics.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"

struct FNOrganLockedData;
class UNOrganComponent;

/**
 * Query bundle passed into FNOrganGraphBuilderContext::FilterCellInputData to narrow down candidate cells.
 */
struct FNCellInputDataFilter
{
	/** When true, only cells flagged bCanBeStartNode are eligible. */
	bool bRequireStart = false;

	/** When true, only cells flagged bCanBeEndNode are eligible. */
	bool bRequireEnd = false;

	/** Cell the filter is stepping away from; used for distance/uniqueness checks. */
	FNCellInputData* SourceCellInputData = nullptr;

	/** Graph node the filter is stepping away from; used for spatial checks. */
	FNProcGenGraphCellNode* SourceCellNode = nullptr;

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
class FNOrganContext
{
	friend struct FNOrganGeneratorBuildGraphTask;

public:

	/** Lower bound on cell count; -1 disables the check. */
	int32 MinimumCellCount = -1;

	/** Upper bound on cell count; -1 disables the check. */
	int32 MaximumCellCount = -1;

	/** Number of retries this organ is allowed before giving up. */
	int32 MaximumRetryCount = 0;

	/** When true, the graph may extend past Bounds. */
	bool bUnbounded = false;

	/** Spatial bounds the graph must stay within unless bUnbounded. */
	FBoxSphereBounds Bounds;

	/** World origin of the organ used as the graph root. */
	FVector Origin;

	/** Bones the builder will anchor the graph on. */
	TArray<FNBoneInputData> BoneInputData;

	/** Candidate cells the builder may pull from. */
	TArray<FNCellInputData> CellInputData;

	/** Output graph, owned by this context until handed off to the task-graph context. */
	TUniquePtr<FNProcGenGraph> CellGraph = nullptr;

	FNOrganContext(const FNOrganLockedData* GeneratorContextMap, uint64 TaskSeed, FString TaskName);
	~FNOrganContext();

	/** @return Deterministic seed used to drive this organ's random stream. */
	uint64 GetSeed() const { return Seed; };

	/** @return Human-readable task name for logs/debug. */
	const FString& GetName() { return Name; };

	/** @return true once the builder has produced a valid graph (cell count within bounds, etc). */
	bool IsSuccessful() const { return bSuccessful; };

	/** @return true if the context is set up well enough to attempt a build. */
	bool IsValid() const { return bIsValid; };

	/** @return true if the current graph satisfies all required invariants (debug helper). */
	bool CheckGraph() const;

	/**
	 * Filter CellInputData into a weighted candidate list for selection.
	 * @param Filter Query constraints (socket size, start/end requirements, source node).
	 * @param CellIndices [out] Indices into CellInputData weighted by their configured weighting.
	 * @param JunctionIndices [out] For each cell index, the subset of its junction keys that match the filter.
	 */
	void FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices);

	/**
	 * Drop the current graph and bump the retry counter.
	 * @param Analytics Accumulated analytics from the just-failed attempt; folded into the retry log.
	 * @return true if another retry is allowed; false once MaximumRetryCount is exhausted.
	 */
	bool ResetForRetry(FNOrganGraphBuilderAnalytics& Analytics);

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
