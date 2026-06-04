// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Contexts/NVirtualOrganContext.h"

#include "NWorldAssemblyMinimal.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Organ/NOrganComponent.h"

namespace
{
	// 180 degrees about Up, written directly as a quaternion to skip the per-call axis-angle sincos.
	// FQuat(FVector::UpVector, PI) evaluates to (Up * sin(PI/2), cos(PI/2)) == (0, 0, 1, 0) exactly.
	const FQuat YawFlipQuat(0.0, 0.0, 1.0, 0.0);
}

FNVirtualOrganContext::FNVirtualOrganContext(const FNWorldOrganData* WorldOrganContext, const uint64 TaskSeed, FString TaskName)
	: Seed(TaskSeed), Name(TaskName)
{
	// This is our last chance to read anything off the main-thread
	//TODO: There is a Seed on the component? What do we do here with it? 
	
	// Cache out some settings
	MinimumCellCount = WorldOrganContext->SourceComponent->MinimumCellCount;
	MaximumCellCount = WorldOrganContext->SourceComponent->MaximumCellCount;
	MaximumRetryCount = WorldOrganContext->MaximumRetryCount;
	BadStartLimit = WorldOrganContext->BadStartLimit;
	
	// Seed override
	if (WorldOrganContext->SourceComponent->Seed != -1)
	{
		Seed = WorldOrganContext->SourceComponent->Seed;
	}
	
	CellHullPenetration = WorldOrganContext->CellHullPenetration;
	WorldHullPenetration = WorldOrganContext->WorldHullPenetration;
	VoxelSize = WorldOrganContext->VoxelSize;
	
	// Keep a local copy of this here
	bUnbounded = WorldOrganContext->SourceComponent->bUnbounded;
	
	// We are going to establish some base understanding of the space, specifically its world origin as well as the bounds.
	Bounds = WorldOrganContext->Bounds;
	Origin = WorldOrganContext->Origin;
	
	// Build a safe reference to all the data so we can operate off-thread without issue
	TMap<TObjectPtr<UNCell>, FNTissueEntry> TissueMap;
	CellInputDataSummary = FNVirtualCellDataSummary();
	
	WorldOrganContext->SourceComponent->GetTissueMap(TissueMap, CellInputDataSummary.GroupTags);
	
	// We're going to process the input cells to see what behavior we are going to be exercising down the road.
	
	// Validate the tissue map has a start
	bool bFoundSomeCells = false;
	
	for (auto Pair : TissueMap)
	{
		if (Pair.Value.MaximumCount > 0 || Pair.Value.MaximumCount == -1)
		{
			bFoundSomeCells = true;
		}
		
		// bFoundStarterTagged
		if (!CellInputDataSummary.bFoundStarterTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_Behavior_Starter))
		{
			CellInputDataSummary.bFoundStarterTagged = true;
		}
		
		// bFoundStarterOnlyTagged
		if (!CellInputDataSummary.bFoundStarterOnlyTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_Behavior_StarterOnly))
		{
			CellInputDataSummary.bFoundStarterOnlyTagged = true;
		}
		
		// bFoundFinisherTagged
		if (!CellInputDataSummary.bFoundFinisherTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_Behavior_Finisher))
		{
			CellInputDataSummary.bFoundFinisherTagged = true;
		}
		
		// bFoundFinisherOnlyTagged
		if (!CellInputDataSummary.bFoundFinisherOnlyTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_Behavior_FinisherOnly))
		{
			CellInputDataSummary.bFoundFinisherOnlyTagged = true;
		}
		
		// Early out as there is nothing else we really are looking for
		if (bFoundSomeCells && 
			CellInputDataSummary.bFoundStarterTagged && 
			CellInputDataSummary.bFoundStarterOnlyTagged && 
			CellInputDataSummary.bFoundFinisherTagged && 
			CellInputDataSummary.bFoundFinisherOnlyTagged)
		{
			break;
		}
	}

	if (!bFoundSomeCells)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as the it appears that no UNCells would be generated from the provided UNTissues (check MaximumCount). Skipping."));
		return;
	}
	
	// Build CellInputData
	CellInputData.Reserve(TissueMap.Num());
	for (const auto& Cell : TissueMap)
	{
		FNVirtualCellData CellDetails;
		CellDetails.AssemblyTags = Cell.Value.AssemblyTags;
		CellDetails.ContextTagsRequired = Cell.Value.RequiredContextTags;
		CellDetails.ContextTagsAdded = Cell.Value.AddedContextTags;
		CellDetails.TagCounterConstraints = Cell.Value.TagCounterConstraints;
		CellDetails.TagCounterOperations = Cell.Value.TagCounterOperations;
		CellDetails.MinimumCount = Cell.Value.MinimumCount;
		CellDetails.MaximumCount = Cell.Value.MaximumCount;
		CellDetails.Weighting = Cell.Value.Weighting;
		CellDetails.MinimumNodeDistance = Cell.Value.MinimumNodeDistance;
		CellDetails.MinimumNodeDepth = Cell.Value.MinimumNodeDepth;
		
		// We won't touch this till later
		CellDetails.Template = Cell.Key;
		
		Cell.Key->Root.CopyTo(CellDetails.CellDetails);
		for (const TPair<int32, FNCellJunctionDetails>& Junction :  Cell.Key->Junctions)
		{
			FNCellJunctionDetails& VirtualJunction = CellDetails.Junctions.Add(Junction.Key, Junction.Value);

			// Precompute the inverse junction quaternion once here; the authored WorldRotation never changes for
			// the lifetime of this virtual data (it survives ResetForRetry), so the per-candidate rotation gate
			// can read it straight back instead of redoing the rotator->quat->inverse conversion every filter pass.
			VirtualJunction.CachedInverseWorldQuat = Junction.Value.WorldRotation.Quaternion().Inverse();
		}

		// Record once whether the pool uses these features at all, so FilterCellInputData can skip the matching
		// gate entirely on builds where no cell could ever be affected by it.
		if (!CellInputDataSummary.bAnyContextTagsRequired && !CellDetails.ContextTagsRequired.IsEmpty())
		{
			CellInputDataSummary.bAnyContextTagsRequired = true;
		}
		if (!CellInputDataSummary.bAnyTagCounterConstraints && CellDetails.TagCounterConstraints.Num() > 0)
		{
			CellInputDataSummary.bAnyTagCounterConstraints = true;
		}

		CellInputData.Add(CellDetails); // TODO: Check this is a Move
	}
	
	// Need to convert the bone data to something were going to use
	BoneInputData.Reserve(WorldOrganContext->ContainedBones.Num());
	for (const auto& Bone : WorldOrganContext->ContainedBones)
	{
		FNVirtualBoneData BoneDetails;
		
		BoneDetails.WorldPosition = Bone->SourceComponent->GetComponentLocation();
		BoneDetails.WorldRotation = Bone->SourceComponent->GetComponentRotation();
		
		BoneDetails.CornerPoints = Bone->CornerPoints;		
		
		BoneDetails.SocketSize = Bone->SourceComponent->SocketSize;
		
		BoneInputData.Add(BoneDetails);
	}
	
	// Validate that we do have bones
	if (BoneInputData.IsEmpty())
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as no UNBoneComponents were provided or found. Skipping."));
		return;
	}
	
	// TODO: handle more then 1 bone
	FNCellInputDataFilter PreFilter;
	PreFilter.NodeDepth = 0;
	PreFilter.SocketSize = BoneInputData[0].SocketSize;
	PreFilter.SourceQuat = FQuat(BoneInputData[0].WorldRotation);
	PreFilter.bIsStartNode = true;
	
	FNWeightedIntegerArray PreIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	
	FilterCellInputData(PreFilter, PreIndices, ValidJunctions);

	if (PreIndices.WeightedCount() == 0)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as no starting junctions are sized to the provided UNBoneComponent."));
		return;
	}
	
	
	// We got to the end so we have validated things so far at this point.
	bIsValid = true;
}

FNVirtualOrganContext::~FNVirtualOrganContext()
{
	if (CellGraph != nullptr)
	{
		CellGraph.Reset();
	}
}

bool FNVirtualOrganContext::CheckGraph()
{
	// We're going to look over all the nodes
	int32 CellNodeCount = 0;
	
	// If we don't have a graph it's a fail
	if (!CellGraph)
	{
		N_VIRTUAL_ORGAN_MESSAGE("CheckGraph FAILED: Graph was null.")
		return false;
	}
	
	for (const auto Pair : CellGraph->GetNodes())
	{
		if (Pair->GetNodeType() == ENAssemblyGraphNodeType::Cell)
		{
			CellNodeCount++;
		}
	}

	// Enforce check for the minimum amount of cells wanted
	if (MinimumCellCount > 0 && CellNodeCount < MinimumCellCount)
	{
		N_VIRTUAL_ORGAN_MESSAGE(FString::Printf(
			TEXT("CheckGraph FAILED: CellNodeCount(%i) < MinimumCellCount(%i)"), CellNodeCount, MinimumCellCount))
		return false;
	}
	
	// Enforce check for maximum amount of cells wanted
	if (MaximumCellCount > 0 && CellNodeCount > MaximumCellCount)
	{
		N_VIRTUAL_ORGAN_MESSAGE(FString::Printf(
			TEXT("CheckGraph FAILED: CellNodeCount(%i) > MaximumCellCount(%i)"), CellNodeCount, MaximumCellCount))
		return false;
	}
	
	// We need to check that each of the RequiredAny groups are met
	if (!CellInputDataSummary.GroupTags.RequiredAnyTags.IsEmpty() && 
		!PlacedTagGroups.RequiredAnyTags.HasAllExact(CellInputDataSummary.GroupTags.RequiredAnyTags))
	{
		// Could log analytics about rejection based on required
		N_VIRTUAL_ORGAN_MESSAGE(FString::Printf(
			TEXT("CheckGraph FAILED: HasAllRequiredAnyTags(%s) != PlacedGroupTags(%s)"), 
			*CellInputDataSummary.GroupTags.RequiredAnyTags.ToStringSimple(), *PlacedTagGroups.RequiredAnyTags.ToStringSimple()))
		return false;
	}
	
	// TODO: We need to check the input cell data minimum counts (but ignore for unique/required)
	

	return true;
}

bool FNVirtualOrganContext::ValidateGraph()
{
	bSuccessful = CheckGraph();
	if (!bSuccessful)
	{
		N_VIRTUAL_ORGAN_MESSAGE("ValidateGraph FAILED!");
	}
	else
	{
		N_VIRTUAL_ORGAN_MESSAGE("ValidateGraph Passed.");
	}
	return bSuccessful;
}

FGameplayTagContainer FNVirtualOrganContext::ResolveSourceBadNeighborTags(const FNTissueTagGroups& GroupTags, FNAssemblyGraphCellNode* SourceCellNode)
{
	FGameplayTagContainer SourceBadNeighborTags;
	if (SourceCellNode != nullptr && !GroupTags.BadNeighborsTags.IsEmpty())
	{
		SourceBadNeighborTags = SourceCellNode->GetAssemblyTags().FilterExact(GroupTags.BadNeighborsTags);
	}
	return SourceBadNeighborTags;
}

bool FNVirtualOrganContext::IsBadNeighbor(const FGameplayTagContainer& SourceBadNeighborTags, const FNVirtualCellData& Candidate)
{
	return !SourceBadNeighborTags.IsEmpty() && Candidate.AssemblyTags.HasAny(SourceBadNeighborTags);
}

bool FNVirtualOrganContext::IsGatedByStarterTags(const bool bIsStartNode, const FNVirtualCellDataSummary& Summary, const FGameplayTagContainer& CandidateTags)
{
	// When the pool contains start-specific content, a start-node candidate must carry one of those tags.
	if (bIsStartNode && (Summary.bFoundStarterTagged || Summary.bFoundStarterOnlyTagged) &&
		!CandidateTags.HasTag(NWorldAssembly_Behavior_Starter) && !CandidateTags.HasTag(NWorldAssembly_Behavior_StarterOnly))
	{
		return true;
	}

	// StarterOnly cells may never be placed anywhere but the start node.
	if (!bIsStartNode && CandidateTags.HasTag(NWorldAssembly_Behavior_StarterOnly))
	{
		return true;
	}

	// NotStarter cells may never be the start node.
	return bIsStartNode && CandidateTags.HasTag(NWorldAssembly_Behavior_NotStarter);
}

bool FNVirtualOrganContext::IsGatedByFinisherTags(const bool bIsEndNode, const FNVirtualCellDataSummary& Summary, const FGameplayTagContainer& CandidateTags)
{
	// When the pool contains end-specific content, an end-node candidate must carry one of those tags.
	if (bIsEndNode && (Summary.bFoundFinisherTagged || Summary.bFoundFinisherOnlyTagged) &&
		!CandidateTags.HasTag(NWorldAssembly_Behavior_Finisher) && !CandidateTags.HasTag(NWorldAssembly_Behavior_FinisherOnly))
	{
		return true;
	}

	// FinisherOnly cells may never be placed anywhere but the end node.
	if (!bIsEndNode && CandidateTags.HasTag(NWorldAssembly_Behavior_FinisherOnly))
	{
		return true;
	}

	// NotFinisher cells may never be the end node.
	return bIsEndNode && CandidateTags.HasTag(NWorldAssembly_Behavior_NotFinisher);
}

bool FNVirtualOrganContext::IsGatedByTagCounterConstraints(const FNVirtualCellData& Candidate, const FNGameplayTagCounter& TagCounter)
{
	// The candidate is only eligible when every one of its constraints passes against the current counter state.
	for (const FNGameplayTagCounterConstraint& Constraint : Candidate.TagCounterConstraints)
	{
		if (!Constraint.DoesPassComparison(TagCounter))
		{
			return true;
		}
	}
	return false;
}

bool FNVirtualOrganContext::IsGatedByMinimumNodeDepth(const int32 MinimumNodeDepth, const int32 SourceNodeDepth)
{
	return MinimumNodeDepth > 0 && MinimumNodeDepth > SourceNodeDepth;
}

FRotator FNVirtualOrganContext::GetRequiredJunctionRotation(const FQuat& SourceQuat, const FRotator& JunctionWorldRotation)
{
	// Flip 180 around Up to oppose the source's facing direction, then undo the junction's local rotation so only
	// the delta we need to apply to the cell remains. The hot path precomputes both terms; this convenience overload
	// composes them on the spot for callers (and tests) holding the raw source quat and authored junction rotation.
	return GetRequiredJunctionRotationPrepared(SourceQuat * YawFlipQuat, JunctionWorldRotation.Quaternion().Inverse());
}

FRotator FNVirtualOrganContext::GetRequiredJunctionRotationPrepared(const FQuat& SourceFlippedQuat, const FQuat& JunctionInverseQuat)
{
	// FQuat::Rotator() already returns each axis pre-normalized (pitch in [-90,90] from asin, yaw/roll in (-180,180]
	// from atan2), and FRotator::NormalizeAxis is a bit-exact identity over that range, so no further normalization
	// is needed here before feeding the matching-rotation constraints.
	return (SourceFlippedQuat * JunctionInverseQuat).Rotator();
}

bool FNVirtualOrganContext::IsGatedByJunctionRotation(const FQuat& SourceQuat, const FRotator& JunctionWorldRotation,
	const FNRotationConstraints& CellConstraints, const FNRotationConstraints& JunctionConstraints)
{
	return IsGatedByJunctionRotationPrepared(SourceQuat * YawFlipQuat, JunctionWorldRotation.Quaternion().Inverse(),
		CellConstraints, JunctionConstraints);
}

bool FNVirtualOrganContext::IsGatedByJunctionRotationPrepared(const FQuat& SourceFlippedQuat, const FQuat& JunctionInverseQuat,
	const FNRotationConstraints& CellConstraints, const FNRotationConstraints& JunctionConstraints)
{
	const FRotator Required = GetRequiredJunctionRotationPrepared(SourceFlippedQuat, JunctionInverseQuat);

	// Both the cell and the junction get a veto: either can independently disable enforcement, but whichever side
	// has bEnforceMatchingRotation set must have the required rotation fall inside its Min/Max range.
	return !CellConstraints.IsMatchingRotationAllowed(Required.Roll, Required.Pitch, Required.Yaw) ||
		!JunctionConstraints.IsMatchingRotationAllowed(Required.Roll, Required.Pitch, Required.Yaw);
}

void FNVirtualOrganContext::FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices)
{
	CellIndices.Reset();
	JunctionIndices.Reset();

	// Resolve which bad-neighbor groups the source cell belongs to once up front; any candidate sharing one of
	// these groups cannot be placed beside it. Stays empty when there is no source node (e.g. the start-node
	// pre-filter) or when no bad-neighbor groups are configured, which skips the per-candidate check below.
	const FGameplayTagContainer SourceBadNeighborTags = ResolveSourceBadNeighborTags(CellInputDataSummary.GroupTags, Filter.SourceCellNode);

	// Unique-tag gating only bites once something carrying a unique tag has actually been placed; hoist the
	// emptiness test out of the per-candidate loop so an empty set costs one check instead of one-per-candidate.
	const bool bCheckUniqueTags = !PlacedTagGroups.UniqueTags.IsEmpty();

	// The source side of the junction-rotation math is invariant for the whole call, so compose it once here:
	// SourceQuat flipped 180 around Up. The per-junction term is read from the junction's CachedInverseWorldQuat.
	const FQuat SourceFlippedQuat = Filter.SourceQuat * YawFlipQuat;

	for (int32 i = 0; i < CellInputData.Num(); i++)
	{
		const FNVirtualCellData* CellData = &CellInputData[i];

		// Early out on some simple filters
		if (!CellData->IsValidSelection(Filter.SocketSize)) continue;


		// CONTEXT TAGS
		if (CellInputDataSummary.bAnyContextTagsRequired &&
			!CellData->ContextTagsRequired.IsEmpty() && !ContextTags.HasAllExact(CellData->ContextTagsRequired))
		{
			continue;
		}

		// CHECK UNIQUE TAGS
		if (bCheckUniqueTags && PlacedTagGroups.UniqueTags.HasAnyExact(CellData->AssemblyTags))
		{
			continue;
		}

		// GATE BASED ON TagCounter Constraints
		if (CellInputDataSummary.bAnyTagCounterConstraints && IsGatedByTagCounterConstraints(*CellData, TagCounter))
		{
			continue;
		}
		
		// FILTER BAD NEIGHBORS
		// Reject candidates that belong to a bad-neighbor group the source cell is also a member of — those
		// two cells are not allowed to be placed beside each other.
		if (IsBadNeighbor(SourceBadNeighborTags, *CellData))
		{
			continue;
		}

		// STARTER TAGS
		if (IsGatedByStarterTags(Filter.bIsStartNode, CellInputDataSummary, CellData->AssemblyTags))
		{
			continue;
		}

		// FINISHER TAGS
		if (IsGatedByFinisherTags(Filter.bIsEndNode, CellInputDataSummary, CellData->AssemblyTags))
		{
			continue;
		}

		// Check minimum distance in current graph for reusing the cell piece
		if (Filter.SourceCellInputData != nullptr && Filter.SourceCellNode != nullptr &&
			CellData->MinimumNodeDistance >= 1 &&
			Filter.SourceCellNode->SearchForMatchingCellInputData(CellData, CellData->MinimumNodeDistance - 1))
		{
			continue;
		}
		
		// Gate by minimum depth. Filter.NodeDepth is the SOURCE node's depth; the candidate lands one hop
		// deeper. Graph depth is rooted at the bone, so the start cell is NodeDepth 1 (hop 0 from start) and
		// "source depth" already equals "candidate's hop-count from the start cell" — the two offsets cancel.
		// A candidate is therefore correctly first eligible at hop == MinimumNodeDepth. The comparison lives in
		// IsGatedByMinimumNodeDepth (do NOT add a +1 there). Covered by NMinimumNodeDepthTests.cpp.
		if (IsGatedByMinimumNodeDepth(CellData->MinimumNodeDepth, Filter.NodeDepth))
		{
			continue;
		}
		
		
		const FNRotationConstraints& CellRotationConstraints = CellData->CellDetails.RotationConstraints;
		
		// Parse Junctions
		TArray<int32> GoodJunctions;
		for (auto Pair : CellData->Junctions)
		{
			if (Pair.Value.SocketSize == Filter.SocketSize)
			{
				// Determine the rotation this junction would have to take on to match Filter.SourceQuat, then veto it
				// against both the cell's and the junction's matching-rotation constraints. Both rotation terms are
				// precomputed (SourceFlippedQuat hoisted above, CachedInverseWorldQuat built with the virtual data).
				// Covered by NJunctionRotationTests.cpp.
				if (IsGatedByJunctionRotationPrepared(SourceFlippedQuat, Pair.Value.CachedInverseWorldQuat, CellRotationConstraints, Pair.Value.RotationConstraints))
				{
					continue;
				}

				GoodJunctions.Add(Pair.Key);
			}
		}
		
		if (GoodJunctions.Num() > 0)
		{
			// Add weighting
			CellIndices.Add(i, CellData->Weighting);
			
			// Fill out selectable junctions
			TArray<int32>& Junctions = JunctionIndices.Add(i, TArray<int32>());
			for (auto Index : GoodJunctions)
			{
				Junctions.Add(Index);
			}
		}
	}
}

bool FNVirtualOrganContext::ResetForRetry()
{
	// Reset counters
	for (int32 i = 0; i < CellInputData.Num(); i++)
	{
		CellInputData[i].UsedCount = 0;
	}
	
	// Drop everything this attempt placed so a retry starts from the same state as the first attempt.
	// PlacedTagGroups is a pure accumulator (starts empty, filled as cells are placed), so it clears to
	// empty; ContextTags and TagCounter genuinely restore to the bases they were seeded from.
	PlacedTagGroups = FNTissueTagGroups();
	ContextTags     = BaseContextTags;
	TagCounter      = BaseTagCounter;
		
	// Clear Graph
	if (CellGraph != nullptr)
	{
		CellGraph.Reset();
	}
	
	RetryCount++;
#if !UE_BUILD_SHIPPING	
	Messages.Reset();
#endif // !UE_BUILD_SHIPPING	
	
	if (RetryCount > MaximumRetryCount)
	{
		return false;
	}
	return true;
}
