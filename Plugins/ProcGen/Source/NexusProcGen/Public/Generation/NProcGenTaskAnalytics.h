// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Analytics/NOrganGraphBuilderAnalytics.h"
#include "Analytics/NProcessPassAnalytics.h"
#include "Analytics/NProcGenTaskTimer.h"
#include "Analytics/NSpawnCellProxiesAnalytics.h"
#include "Developer/NReport.h"

/** Member name used by every N_PROCEDURAL_GENERATION_ANALYTICS_* macro for the analytics shared pointer. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR AnalyticsPtr
/** Member name used by index-based macros for the per-task analytics record index. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_INDEX AnalyticsIndex

#if !UE_BUILD_SHIPPING
/** Allocate the analytics shared pointer for the current Operation; expected to expand inside an owner that has an `Operation` in scope. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_CREATE N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR = MakeShared<FNProcGenTaskAnalytics, ESPMode::ThreadSafe>(Operation->GetDisplayName());
/** Append the analytics shared pointer to a constructor parameter list. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR , const TSharedPtr<FNProcGenTaskAnalytics>& N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR
/** Append the analytics initializer to a constructor's member initializer list. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER , N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR(N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR.ToSharedRef())
/** Append the analytics shared pointer when forwarding it through an existing argument list. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_CLASS_REF , N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR
/** Declare the analytics shared pointer as a class member. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_PTR	TSharedPtr<FNProcGenTaskAnalytics> N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR;
/** Declare the analytics shared reference as a class member. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF TSharedRef<FNProcGenTaskAnalytics> N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR;
/** Reset the analytics shared pointer once the owner is finished with it. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_DELETE N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR.Reset();
/** Declare a local index used to address the per-task analytics record on the analytics object. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_LOCAL int32 N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_INDEX = -1;

/** Forward a no-argument analytics call. */
#define N_PROCEDURAL_GENERATION_ANALYTICS(Method) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method();
/** Forward a one-argument analytics call. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_ONE_PARAM(Method, Parameter) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method(Parameter);
/** Forward a two-argument analytics call. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_TWO_PARAM(Method, Parameter1, Parameter2) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method(Parameter1, Parameter2);
/** Forward a three-argument analytics call. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_THREE_PARAM(Method, Parameter1, Parameter2, Parameter3) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method(Parameter1, Parameter2, Parameter3);
/** Forward a four-argument analytics call. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_FOUR_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method(Parameter1, Parameter2, Parameter3, Parameter4);
/** Forward a five-argument analytics call. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_FIVE_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4, Parameter5) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method(Parameter1, Parameter2, Parameter3, Parameter4, Parameter5);
/** Forward an analytics call that takes the previously-stashed analytics record index. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX(Method) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method(N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_INDEX);
/** Forward an analytics call that returns a record index and stash the result into the existing index member. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_SET(Method) \
	N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_INDEX = N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method();
/** Define a new const local index initialized from an analytics call that returns a record index. */
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_DEFINE(Method) \
	const int32 N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_INDEX = N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->Method();
#else
#define N_PROCEDURAL_GENERATION_ANALYTICS_CREATE
#define N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR
#define N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER
#define N_PROCEDURAL_GENERATION_ANALYTICS_CLASS_REF
#define N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_PTR
#define N_PROCEDURAL_GENERATION_ANALYTICS_SHARED_REF
#define N_PROCEDURAL_GENERATION_ANALYTICS_DELETE
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_LOCAL

#define N_PROCEDURAL_GENERATION_ANALYTICS(Method)
#define N_PROCEDURAL_GENERATION_ANALYTICS_ONE_PARAM(Method, Parameter)
#define N_PROCEDURAL_GENERATION_ANALYTICS_TWO_PARAM(Method, Parameter1, Parameter2)
#define N_PROCEDURAL_GENERATION_ANALYTICS_THREE_PARAM(Method, Parameter1, Parameter2, Parameter3)
#define N_PROCEDURAL_GENERATION_ANALYTICS_FOUR_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4)
#define N_PROCEDURAL_GENERATION_ANALYTICS_FIVE_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4, Parameter5)
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX(Method)
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_SET(Method)
#define N_PROCEDURAL_GENERATION_ANALYTICS_INDEX_DEFINE(Method)
#endif // !UE_BUILD_SHIPPING

/**
 * Aggregates timing and counter analytics for one UNProcGenOperation's full task-graph build.
 *
 * Each task-graph stage calls Start/Finish helpers as it runs; the per-organ and per-pass stages
 * additionally allocate a record up front via *Create() and address it by the returned index for
 * the rest of the build. Reports are produced once everything has drained via GetTimespanReport
 * and GetCountersReport. The whole class is compiled out in shipping builds — see the macros above.
 *
 * @note Calls into the various Start/Finish methods may originate from worker threads; instances
 * should be held as TSharedPtr<FNProcGenTaskAnalytics, ESPMode::ThreadSafe>.
 */
class FNProcGenTaskAnalytics
{
public:
	explicit FNProcGenTaskAnalytics(const FText& DisplayName) : DisplayName(DisplayName) {};

	/** Start the task-graph creation timer. */
	void TaskGraphCreationStart();
	/** Stop the task-graph creation timer. */
	void TaskGraphCreationFinish();

	/** Start the timer for the create-virtual-world stage. */
	void CreateVirtualWorldContextStart();
	/** Stop the timer for the create-virtual-world stage. */
	void CreateVirtualWorldContextFinish();

	/** Start the timer for the process-virtual-world stage. */
	void ProcessVirtualWorldContextStart();
	/** Stop the timer for the process-virtual-world stage. */
	void ProcessVirtualWorldContextFinish();

	/** Allocate a new organ-graph-builder analytics record and return its index. */
	int32 OrganGraphBuilderCreate();
	/** Mark the start time of the organ-graph-builder record at Index. */
	void OrganGraphBuilderStart(int32 Index);
	/** Mark the finish time of the organ-graph-builder record at Index. */
	void OrganGraphBuilderFinish(int32 Index);

	/**
	 * Populate the organ-graph-builder record at Index with task configuration captured up front.
	 * @param Index Record index returned from OrganGraphBuilderCreate.
	 * @param Name Human-readable task name for logs.
	 * @param MinimumCellCount Lower bound on cell count required by the build.
	 * @param MaximumCellCount Upper bound on cell count required by the build.
	 * @param MaximumRetryCount Number of retry iterations the builder is allowed.
	 */
	void OrganGraphBuilder_Init(int32 Index, const FString& Name, int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount);

	/** Increment the AddNullNodes counter for the current iteration of the record at Index. */
	void OrganGraphBuilder_AddNullNode(int32 Index);
	/** Increment the AddCellNodes counter for the current iteration of the record at Index. */
	void OrganGraphBuilder_AddCellNode(int32 Index);

	/** Increment the discard-out-of-bounds-start counter for the record at Index. */
	void OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index);
	/** Increment the discard-world-colliding-start counter for the record at Index. */
	void OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index);

	/** Increment the discard-out-of-bounds-cell counter for the record at Index. */
	void OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index);
	/** Increment the discard-intersecting-cell counter for the record at Index. */
	void OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index);
	/** Increment the discard-world-colliding-cell counter for the record at Index. */
	void OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index);
	/** Increment the cell-vs-existing-node collision discard counter for the record at Index. */
	void OrganGraphBuilder_DiscardExistingNodeWorldCollidingCellNode(int32 Index);

	/** Advance the per-iteration counters in the organ-graph-builder record at Index. */
	void OrganGraphBuilder_NextIteration(int32 Index);

	/** Allocate a new collect-generation-passes analytics record and return its index. */
	int32 CollectGenerationPassesCreate();
	/** Mark the start time of the collect-generation-passes record at Index. */
	void CollectGenerationPassesStart(int32 Index);
	/** Mark the finish time of the collect-generation-passes record at Index. */
	void CollectGenerationPassesFinish(int32 Index);

	/** Start the timer for the create-spawn-cells-context stage. */
	void CreateSpawnCellsContextStart();
	/** Stop the timer for the create-spawn-cells-context stage. */
	void CreateSpawnCellsContextFinish();

	/** Allocate a new spawn-cell-proxies analytics record and return its index. */
	int32 SpawnCellProxiesCreate();
	/** Mark the start time of the spawn-cell-proxies record at Index. */
	void SpawnCellProxiesStart(int32 Index);
	/** Append Template to the list of cells spawned during the record at Index. */
	void SpawnCellProxiesSpawned(int32 Index, FName Template);
	/** Mark the finish time of the spawn-cell-proxies record at Index. */
	void SpawnCellProxiesFinish(int32 Index);

	/** Start the timer for the procgen-finalize stage. */
	void ProcGenFinalizeStart();
	/** Stop the timer for the procgen-finalize stage. */
	void ProcGenFinalizeFinish();

	/** @return Multi-line text report listing the duration of each tracked stage. */
	FString GetTimespanReport();
	/** @return Multi-line text report listing per-iteration counter values for each tracked stage. */
	FString GetCountersReport();

	FNReport GetReport();	
	
private:
	/** Operation display name used as the heading in generated reports. */
	FText DisplayName;

	/** Timer for the task-graph creation stage. */
	FNProcGenTaskTimer TaskGraphCreationTimer = FNProcGenTaskTimer();
	/** Timer for the create-virtual-world stage. */
	FNProcGenTaskTimer CreateVirtualWorldContextTimer = FNProcGenTaskTimer();
	/** Timer for the process-virtual-world stage. */
	FNProcGenTaskTimer ProcessVirtualWorldContextTimer = FNProcGenTaskTimer();

	/** Timer for the create-spawn-cells-context stage. */
	FNProcGenTaskTimer CreateSpawnCellsContextTimer = FNProcGenTaskTimer();
	/** Timer for the procgen-finalize stage. */
	FNProcGenTaskTimer ProcGenFinalizeTimer = FNProcGenTaskTimer();

	/** Per-organ-builder records allocated by OrganGraphBuilderCreate. */
	TArray<FNOrganGraphBuilderAnalytics> OrganGraphBuilderAnalytics;
	/** Per-pass records allocated by CollectGenerationPassesCreate. */
	TArray<FNProcessPassAnalytics> ProcessPassAnalytics;
	/** Per-spawn-cell-proxies records allocated by SpawnCellProxiesCreate. */
	TArray<FNSpawnCellProxiesAnalytics> SpawnCellProxiesAnalytics;
};