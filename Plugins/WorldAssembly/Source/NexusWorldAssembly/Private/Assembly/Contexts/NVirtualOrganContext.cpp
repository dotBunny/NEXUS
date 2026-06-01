// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Contexts/NVirtualOrganContext.h"

#include "NWorldAssemblyMinimal.h"
#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Organ/NOrganComponent.h"

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
		if (!CellInputDataSummary.bFoundStarterTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_BuiltIn_Starter))
		{
			CellInputDataSummary.bFoundStarterTagged = true;
		}
		
		// bFoundStarterOnlyTagged
		if (!CellInputDataSummary.bFoundStarterOnlyTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_BuiltIn_StarterOnly))
		{
			CellInputDataSummary.bFoundStarterOnlyTagged = true;
		}
		
		// bFoundFinisherTagged
		if (!CellInputDataSummary.bFoundFinisherTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_BuiltIn_Finisher))
		{
			CellInputDataSummary.bFoundFinisherTagged = true;
		}
		
		// bFoundFinisherOnlyTagged
		if (!CellInputDataSummary.bFoundFinisherOnlyTagged && Pair.Value.AssemblyTags.HasTag(NWorldAssembly_BuiltIn_FinisherOnly))
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
		CellDetails.OutputTags = Cell.Value.OutputTags;
		CellDetails.MinimumCount = Cell.Value.MinimumCount;
		CellDetails.MaximumCount = Cell.Value.MaximumCount;
		CellDetails.Weighting = Cell.Value.Weighting;
		CellDetails.MinimumNodeDistance = Cell.Value.MinimumNodeDistance;
		CellDetails.MinimumNodeDepth = Cell.Value.MinimumNodeDepth;
		CellDetails.bAlwaysRelevant = Cell.Value.bAlwaysRelevant;
		
		// We won't touch this till later
		CellDetails.Template = Cell.Key;
		
		Cell.Key->Root.CopyTo(CellDetails.CellDetails);
		for (const TPair<int32, FNCellJunctionDetails>& Junction :  Cell.Key->Junctions)
		{
			CellDetails.Junctions.Add(Junction.Key, Junction.Value);
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
	if (!CellGraph) return false;
	
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
		// Could log analytics about rejection based on cell count
		return false;
	}
	
	// Enforce check for maximum amount of cells wanted
	if (MaximumCellCount > 0 && CellNodeCount > MaximumCellCount)
	{
		// Could log analytics about rejection based on cell count
		return false;
	}
	
	// We need to check that each of the RequiredAny groups are met
	if (CellInputDataSummary.GroupTags.HasRequiredAnyTags() && !CellInputDataSummary.GroupTags.HasAllRequiredAnyTags(PlacedTagGroups.GetRequiredAnyTags()))
	{
		// Could log analytics about rejection based on required
		return false;
	}
	
	// TODO: We need to check the input cell data minimum counts (but ignore for unique/required)
	

	return true;
}

bool FNVirtualOrganContext::ValidateGraph()
{
	bSuccessful = CheckGraph();
	return bSuccessful;
}

void FNVirtualOrganContext::FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices)
{
	CellIndices.Empty();
	JunctionIndices.Empty();
	
	for (int32 i = 0; i < CellInputData.Num(); i++)
	{
		const FNVirtualCellData* CellData = &CellInputData[i];
		
		// Early out on some simple filters
		if (!CellData->IsValidSelection(Filter.SocketSize)) continue;
		
		
		// STARTER TAGS
		
		// Handle Starter Nodes
		if (Filter.bIsStartNode && (CellInputDataSummary.bFoundStarterTagged || CellInputDataSummary.bFoundStarterOnlyTagged))
		{
			// We had some tagged content specific to starting off
			if (!CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_Starter) && !CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_StarterOnly))
			{
				// Didn't have what we were looking for
				continue;
			}
		}
		
		// Bail out from list if it is StarterOnly.
		if (!Filter.bIsStartNode && CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_StarterOnly))
		{
			continue;
		}
		
		if (Filter.bIsStartNode && CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_NotStarter))
		{
			continue;
		}
		
		// FINISHER TAGS
		
		// Handle Starter Nodes
		if (Filter.bIsEndNode && (CellInputDataSummary.bFoundFinisherTagged || CellInputDataSummary.bFoundFinisherOnlyTagged))
		{
			// We had some tagged content specific to ending off
			if (!CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_Finisher) && !CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_FinisherOnly))
			{
				// Didn't have what we were looking for
				continue;
			}
		}
		
		// Bail out from list if it is FinisherOnly.
		if (!Filter.bIsEndNode && CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_FinisherOnly))
		{
			continue;
		}
		
		if (Filter.bIsEndNode && CellData->AssemblyTags.HasTag(NWorldAssembly_BuiltIn_NotFinisher))
		{
			continue;
		}
		
		// CHECK UNIQUE TAGS
		if (PlacedTagGroups.HasAnyUniqueTags(CellData->AssemblyTags))
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
		// A candidate is therefore correctly first eligible at hop == MinimumNodeDepth. Do NOT add a +1 here;
		// it would let cells appear one hop too early. Covered by NMinimumNodeDepthTests.cpp.
		if (CellData->MinimumNodeDepth > 0 &&
			CellData->MinimumNodeDepth > Filter.NodeDepth)
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
				// Determine the rotation this junction would have to take on to match Filter.SourceQuat. Same math used in
				// ProcessCellNode when actually placing the cell: flip 180 around Up to oppose the source's facing direction,
				// then undo the junction's local rotation so only the delta we need to apply to the cell remains.
				const FQuat TargetJunctionLocalQuat = Pair.Value.WorldRotation.Quaternion();
				const FQuat RequiredRotationQuat = Filter.SourceQuat * FQuat(FVector::UpVector, PI) * TargetJunctionLocalQuat.Inverse();
				const FRotator RequiredRotation = RequiredRotationQuat.Rotator();
				const FNRotationConstraints& JunctionRotationConstraints = Pair.Value.RotationConstraints;
				
				const float RequiredRoll  = FRotator::NormalizeAxis(RequiredRotation.Roll);
				const float RequiredPitch = FRotator::NormalizeAxis(RequiredRotation.Pitch);
				const float RequiredYaw   = FRotator::NormalizeAxis(RequiredRotation.Yaw);

				// Both the cell and the junction get a veto: either can independently disable enforcement, but whichever side
				// has bEnforceMatchingRotation set must have the required rotation fall inside its Min/Max range.
				if (!CellRotationConstraints.IsMatchingRotationAllowed(RequiredRoll, RequiredPitch, RequiredYaw) ||
					!JunctionRotationConstraints.IsMatchingRotationAllowed(RequiredRoll, RequiredPitch, RequiredYaw))
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
	
	// Reset placed tags
	PlacedTagGroups = FNTissueTagGroups();
		
	// Clear Graph
	if (CellGraph != nullptr)
	{
		CellGraph.Reset();
	}
	
	RetryCount++;
	
	if (RetryCount > MaximumRetryCount)
	{
		return false;
	}
	return true;
}
