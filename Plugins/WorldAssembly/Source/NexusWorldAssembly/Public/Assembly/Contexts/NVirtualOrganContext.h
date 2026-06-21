// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblySettings.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Organ/NOrganComponent.h"
#include "Types/NCardinalDirection.h"
#include "Types/NRotationConstraints.h"

#if !UE_BUILD_SHIPPING
#define N_VIRTUAL_ORGAN_MESSAGE(Message) \
	Messages.Add(Message);
#else
#define N_VIRTUAL_ORGAN_MESSAGE(Message)
#endif // !UE_BUILD_SHIPPING

struct FNWorldOrganData;
class UNOrganComponent;

/**
 * Query bundle passed into FNVirtualOrganContext::FilterCellInputData to narrow down candidate cells.
 */
struct FNCellInputDataFilter
{
	/**
	 * Real NodeDepth of the source node the filter is stepping away from (bone = 0, start cell = 1, each hop +1).
	 * A candidate stepping off this source lands at NodeDepth + 1, which is what the depth gates compare against
	 * each candidate's Minimum/MaximumNodeDepth. 0 means the bone (i.e. selecting the start cell).
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

	FVector WorldPosition;
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

	FGuid Identifier;

	/** Allowed penetration, in world units, between adjacent cell hulls before they are treated as overlapping. */
	float CellHullPenetration = 10.f;

	/** Allowed penetration, in world units, between a placed cell hull and existing world collision before it is rejected. */
	float WorldHullPenetration = 1.f;

	float AssemblyDirectionTolerance = 15.f;

	ENOrganDirectionConstraintMode AssemblyDirectionMode = ENOrganDirectionConstraintMode::StartBone;

	/** World-space size of a single voxel, cached from UNWorldAssemblySettings so placed cells re-voxelize without re-reading settings. */
	FVector VoxelSize = FVector(100.f, 100.f, 100.f);

	/** When false, the organ can produce no results and still be considered a successful assembly operation. */
	bool bRequired = true;

	/** Lower bound on cell count; 0 disables the check. */
	int32 MinimumCellCount = 0;

	/** Upper bound on cell count; 0 disables the check. */
	int32 MaximumCellCount = 0;

	/** Number of retries this organ is allowed before giving up. */
	int32 MaximumRetryCount = 0;

	FGameplayTagContainer RequiredContextTags;

	TArray<FNGameplayTagCounterConstraint> RequiredTagCounters;

	/** When true, the graph may extend past Bounds. */
	bool bUnbound = false;

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

	/** TagCounter present at the start of organ build, used to ensure we can reset our state. */
	FNGameplayTagCounter BaseTagCounter;

	/** TagCounter values currently in-flight of this attempt. */
	FNGameplayTagCounter TagCounter;

	/** Bones the builder will anchor the graph on. */
	TArray<FNVirtualBoneData> BoneInputData;

	/** Candidate cells the builder may pull from. */
	TArray<FNVirtualCellData> CellInputData;

	/** Cached classification summary of CellInputData (starter/finisher availability and tag groups). */
	FNVirtualCellDataSummary CellInputDataSummary;

	/** Output graph, owned by this context until handed off to the task-graph context. */
	TUniquePtr<FNAssemblyGraph> CellGraph = nullptr;

	FNVirtualOrganContext(const FNWorldOrganData* WorldOrganContext, uint64 TaskSeed, FString TaskName);
	NEXUSWORLDASSEMBLY_API ~FNVirtualOrganContext();

#if WITH_TESTS
	/**
	 * Test-only constructor that bypasses the live-component setup the production constructor performs. Every pool,
	 * tag, and graph member is left at its default so a unit test can populate exactly the state it needs and then
	 * exercise the off-thread logic (e.g. ResetForRetry) in isolation. Not compiled outside test builds.
	 * @param TaskSeed Seed the context reports through GetSeed.
	 * @param TaskName Human-readable task name reported through GetName.
	 */
	FNVirtualOrganContext(const uint64 TaskSeed, FString TaskName) : Seed(TaskSeed), Name(MoveTemp(TaskName)) {}
#endif // WITH_TESTS

	/** @return Deterministic seed used to drive this organ's random stream. */
	uint64 GetSeed() const { return Seed; };
#if !UE_BUILD_SHIPPING
	const TArray<FString>& GetMessages() const { return Messages; }
#endif

	/** @return Human-readable task name for logs/debug. */
	const FString& GetName() { return Name; };

	const FGuid& GetIdentifier() const { return Identifier; };

	/** @return true once the builder has produced a valid graph (cell count within bounds, etc.). */
	bool IsSuccessful() const { return bSuccessful; };

	bool IsRequired() const { return bRequired; };

	/** @return true if the context is set up well enough to attempt a build. */
	bool IsValid() const { return bIsValid; };

	/** @return true if the current graph satisfies all required invariants (debug helper). */
	NEXUSWORLDASSEMBLY_API bool CheckGraph();

	/**
	 * Filter CellInputData into a weighted candidate list for selection.
	 * @param Filter Query constraints (socket size, start/end requirements, source node).
	 * @param CellIndices [out] Indices into CellInputData weighted by their configured weighting.
	 * @param JunctionIndices [out] For each cell index, the subset of its junction keys that match the filter.
	 */
	NEXUSWORLDASSEMBLY_API void FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices);

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
	 * @param Cell Candidate cell to evaluate.
	 * @param UniqueAndRequiredTags Intersection of the pool's Unique and RequiredAny group tags; cells matching one
	 *        of these are governed by the RequiredAny check rather than their per-cell minimum.
	 * @return true when Cell is finisher-eligible (Finisher/FinisherOnly) and still short of an enforceable
	 *         MinimumCount — i.e. a cell the finisher-minimum guarantee pass should try to place. The minimum skips
	 *         mirror FNVirtualOrganContext::CheckGraph (non-positive minimum, minimum above a positive maximum, and
	 *         combined Unique + RequiredAny cells).
	 */
	static NEXUSWORLDASSEMBLY_API bool IsUnmetFinisherMinimum(const FNVirtualCellData& Cell, const FGameplayTagContainer& UniqueAndRequiredTags);

	/**
	 * @param Candidate Cell being considered for placement.
	 * @param TagCounter Current tag-counter state to evaluate the candidate's constraints against.
	 * @return true if any of the candidate's TagCounterConstraints fails its comparison and the candidate must be excluded.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByTagCounterConstraints(const FNVirtualCellData& Candidate, const FNGameplayTagCounter& TagCounter);

	/**
	 * Gate a candidate by minimum graph depth. Min/MaxNodeDepth are 1-based depths that match a cell's real
	 * NodeDepth (bone = 0, start cell = 1, each hop +1), so CandidateNodeDepth is the prospective NodeDepth the
	 * candidate would take on (the source node's depth + 1, computed by the caller).
	 * @param MinimumNodeDepth The candidate's configured minimum depth; 0 disables the gate.
	 * @param CandidateNodeDepth The prospective NodeDepth the candidate would land at.
	 * @return true if the candidate is too shallow and must be gated out.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByMinimumNodeDepth(int32 MinimumNodeDepth, int32 CandidateNodeDepth);

	/**
	 * Gate a candidate by maximum graph depth. Mirrors IsGatedByMinimumNodeDepth: Min/MaxNodeDepth are 1-based
	 * depths matching a cell's real NodeDepth (bone = 0, start cell = 1), and CandidateNodeDepth is the prospective
	 * NodeDepth the candidate would land at. The comparison is inclusive: a candidate at exactly MaximumNodeDepth
	 * is still allowed and only deeper candidates are gated.
	 * @param MaximumNodeDepth The candidate's configured maximum depth; 0 disables the gate.
	 * @param CandidateNodeDepth The prospective NodeDepth the candidate would land at.
	 * @return true if the candidate is too deep and must be gated out.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByMaximumNodeDepth(int32 MaximumNodeDepth, int32 CandidateNodeDepth);

	/**
	 * Gate a candidate by its required compass heading. Angle is the candidate's bearing measured from the
	 * configured directional reference point (see FilterCellInputData and AssemblyDirectionMode), and the candidate
	 * survives only when that bearing lands within Tolerance degrees of Direction. Wrapping is handled by
	 * FNCardinalDirectionUtils::IsCloseToDirection, so the 0/360 seam and out-of-range inputs compare correctly.
	 * @param Angle The candidate's bearing in degrees (any range; normalized internally).
	 * @param Direction The cardinal heading the candidate is constrained to.
	 * @param Tolerance Maximum absolute degree deviation (+/-) from Direction that still permits placement.
	 * @return true if the candidate's bearing is outside the tolerance window and must be gated out.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByDirectionalConstraint(float Angle, ENCardinalDirection Direction, float Tolerance);

	/**
	 * Resolve the static world-space reference point a directional constraint measures candidate bearings from, per
	 * the configured mode. StartBone returns the start bone position; OrganCenter returns the organ volume's
	 * geometric center (Bounds.Origin); DynamicCentroid returns the start bone as its pre-placement seed (the live
	 * centroid is resolved per filter pass in FilterCellInputData, not here). Unbound organs have a degenerate
	 * bounds, so OrganCenter falls back to the start bone for them.
	 * @param Mode The configured directional-constraint mode.
	 * @param bUnbound True when the organ is unbounded (its Bounds is a degenerate/near-infinite box).
	 * @param Bounds The organ's spatial bounds; its Origin is the volume's geometric center.
	 * @param StartBoneWorldPosition World position of the start bone (StartBone reference and the unbound/seed fallback).
	 * @return The world-space reference point candidate bearings are measured from.
	 */
	static NEXUSWORLDASSEMBLY_API FVector ResolveDirectionTargetPosition(ENOrganDirectionConstraintMode Mode,
		bool bUnbound, const FBoxSphereBounds& Bounds, const FVector& StartBoneWorldPosition);


	/**
	 * Resolve the rotation a candidate junction must adapt to mate with the source junction. Mirrors the
	 * placement math in ProcessCellNode: flip 180 around Up to oppose the source's facing direction, then undo
	 * the junction's own local rotation so only the delta the cell must apply remains. The result is returned
	 * with each axis normalized to [-180, 180] so it can be fed straight into the matching-rotation constraints.
	 * @param SourceQuat Orientation of the source junction the candidate is mating against.
	 * @param JunctionWorldRotation The candidate junction's authored rotation.
	 * @return The per-axis-normalized rotation the candidate cell must take on for this junction to line up.
	 */
	static NEXUSWORLDASSEMBLY_API FRotator GetRequiredJunctionRotation(const FQuat& SourceQuat, const FRotator& JunctionWorldRotation);

	/**
	 * Hot-path variant of GetRequiredJunctionRotation that takes both terms pre-composed, skipping the per-call
	 * axis-angle flip and rotator->quat conversions. FilterCellInputData hoists SourceFlippedQuat out of its loops
	 * and reads JunctionInverseQuat from FNCellJunctionDetails::CachedInverseWorldQuat.
	 * @param SourceFlippedQuat SourceQuat already multiplied by the 180-around-Up flip.
	 * @param JunctionInverseQuat The candidate junction's authored rotation as a quaternion, inverted.
	 * @return The per-axis-normalized rotation the candidate cell must take on for this junction to line up.
	 */
	static FRotator GetRequiredJunctionRotationPrepared(const FQuat& SourceFlippedQuat, const FQuat& JunctionInverseQuat);

	/**
	 * Gate a candidate junction by the rotation it would have to adapt to mate with the source. Both the cell
	 * and the junction get an independent veto: either may disable enforcement, but whichever side enforces must
	 * have the required rotation (from GetRequiredJunctionRotation) fall inside its matching interval.
	 * @param SourceQuat Orientation of the source junction the candidate is mating against.
	 * @param JunctionWorldRotation The candidate junction's authored rotation.
	 * @param CellConstraints The candidate cell's rotation constraints (cell-wide veto).
	 * @param JunctionConstraints The candidate junction's own rotation constraints.
	 * @return true if the required rotation is disallowed by either constraint set and the junction must be skipped.
	 */
	static NEXUSWORLDASSEMBLY_API bool IsGatedByJunctionRotation(const FQuat& SourceQuat, const FRotator& JunctionWorldRotation,
		const FNRotationConstraints& CellConstraints, const FNRotationConstraints& JunctionConstraints);

	/**
	 * Hot-path variant of IsGatedByJunctionRotation taking both rotation terms pre-composed; see
	 * GetRequiredJunctionRotationPrepared. Used by FilterCellInputData's per-candidate junction loop.
	 * @param SourceFlippedQuat SourceQuat already multiplied by the 180-around-Up flip.
	 * @param JunctionInverseQuat The candidate junction's authored rotation as a quaternion, inverted.
	 * @param CellConstraints The candidate cell's rotation constraints (cell-wide veto).
	 * @param JunctionConstraints The candidate junction's own rotation constraints.
	 * @return true if the required rotation is disallowed by either constraint set and the junction must be skipped.
	 */
	static bool IsGatedByJunctionRotationPrepared(const FQuat& SourceFlippedQuat, const FQuat& JunctionInverseQuat,
		const FNRotationConstraints& CellConstraints, const FNRotationConstraints& JunctionConstraints);

	/**
	 * Drop the current graph and bump the retry counter.
	 * @return true if another retry is allowed; false once MaximumRetryCount is exhausted.
	 */
	NEXUSWORLDASSEMBLY_API bool ResetForRetry();

	/** Run the post-build invariant checks; flips bSuccessful on success. */
	NEXUSWORLDASSEMBLY_API bool ValidateGraph();

	/**
	 * Set the static world-space point the directional constraint measures candidate bearings from. Seeded once per
	 * mode at construction (see ResolveDirectionTargetPosition); DynamicCentroid overrides the effective reference
	 * per filter pass and uses this only as its pre-placement fallback.
	 * @param Location World-space origin for direction-constraint bearings.
	 */
	void SetDirectionTargetPosition(const FVector& Location) { DirectionTargetPosition = Location; }

	/** @return The static world-space point candidate bearings are measured from (the DynamicCentroid fallback). */
	FVector GetDirectionTargetPosition() const { return DirectionTargetPosition; }

	int32 GetRetryCount() const { return RetryCount; }
private:
	/** Number of retries consumed so far. */
	int32 RetryCount = 0;

	/** True once construction inputs have been validated. */
	bool bIsValid = false;

	/** True once ValidateGraph accepts the produced graph. */
	bool bSuccessful = false;

	/** Random-stream seed for this organ's build. */
	uint64 Seed;

	/**
	 * Static world-space point direction-constraint bearings are measured from, seeded per mode at construction.
	 * For DynamicCentroid this is only the pre-placement fallback; the live reference is the graph's cell centroid,
	 * resolved per filter pass in FilterCellInputData.
	 */
	FVector DirectionTargetPosition;

	/** Human-readable task name used in logs. */
	FString Name;
#if !UE_BUILD_SHIPPING
	TArray<FString> Messages;
#endif
};
