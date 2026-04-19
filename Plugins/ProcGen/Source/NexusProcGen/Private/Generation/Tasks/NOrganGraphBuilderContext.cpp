// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NOrganGraphBuilderContext.h"

#include "NProcGenMinimal.h"
#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Generation/NProcGenOperationContext.h"
#include "Organ/NOrganComponent.h"

FNOrganGraphBuilderContext::FNOrganGraphBuilderContext(const FNProcGenOperationOrganContext* GeneratorContextMap, const uint64 TaskSeed, FString TaskName)
	: Seed(TaskSeed), Name(TaskName)
{
	// This is our last chance to read anything off the main-thread
	//TODO: There is a Seed on the component? What do we do here with it?
	
	// Cache out some settings
	MinimumCellCount = GeneratorContextMap->SourceComponent->MinimumCellCount;
	MaximumCellCount = GeneratorContextMap->SourceComponent->MaximumCellCount;
	MaximumRetryCount = GeneratorContextMap->MaximumRetryCount;
	
	bUnbounded = GeneratorContextMap->SourceComponent->bUnbounded;
	
	// We are going to establish some base understanding of the space, specifically its world origin as well as the bounds.
	if (!bUnbounded && GeneratorContextMap->SourceComponent->IsVolumeBased())
	{
		const AVolume* Volume = GeneratorContextMap->SourceComponent->GetVolume();
		
		Bounds = Volume->GetBounds();
		Origin = Volume->GetActorLocation();
	}
	else
	{
		// Unbounded
		Bounds = FBox(FVector(MIN_dbl, MIN_dbl, MIN_dbl), FVector(MAX_dbl, MAX_dbl, MAX_dbl));
		Origin = FVector::ZeroVector;
	}

	
	// Build a safe reference to all the data so we can operate off-thread without issue
	TMap<TObjectPtr<UNCell>, FNTissueEntry> TissueMap = GeneratorContextMap->SourceComponent->GetTissueMap();
	
	// Validate the tissue map has a start
	bool bFoundStartNode = false;
	bool bFoundSomeCells = false;
	
	for (auto Pair : TissueMap)
	{
		if (Pair.Value.MaximumCount > 0 || Pair.Value.MaximumCount == -1)
		{
			bFoundSomeCells = true;
		}
		
		if (Pair.Value.bCanBeStartNode)
		{
			bFoundStartNode = true;
		}
		
		// We've hit our marks, early out.
		if (bFoundSomeCells && bFoundStartNode)
		{
			break;
		}
	}
	
	if (!bFoundStartNode)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as the provided UNTissues do not contain a UNCell flagged capable of being the Start Node. Skipping."))
		return;
	}
	
	if (!bFoundSomeCells)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as the it appears that no UNCells would be generated from the provided UNTissues (check MaximumCount). Skipping."))
		return;
	}
	
	// Build CellInputData
	CellInputData.Reserve(TissueMap.Num());
	for (const auto& Cell : TissueMap)
	{
		FNCellInputData CellDetails;
		
		// TODO: We could implement some checks on the UNCell about cross referencing and what happens?
		CellDetails.MinimumCount = Cell.Value.MinimumCount;
		CellDetails.MaximumCount = Cell.Value.MaximumCount;
		CellDetails.bCanBeStartNode = Cell.Value.bCanBeStartNode;
		CellDetails.bCanBeEndNode = Cell.Value.bCanBeEndNode;
		CellDetails.Weighting = Cell.Value.Weighting;
		
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
	BoneInputData.Reserve(GeneratorContextMap->ContainedBones.Num());
	for (const auto& Bone : GeneratorContextMap->ContainedBones)
	{
		FNBoneInputData BoneDetails;
		
		BoneDetails.WorldPosition = Bone->SourceComponent->GetComponentLocation();
		BoneDetails.WorldRotation = Bone->SourceComponent->GetComponentRotation();
		
		BoneDetails.CornerPoints = Bone->CornerPoints;		
		
		BoneDetails.SocketSize = Bone->SourceComponent->SocketSize;
		
		BoneInputData.Add(BoneDetails);
	}
	
	// Validate that we do have bones
	if (BoneInputData.Num() == 0)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as no UNBoneComponents were provided or found. Skipping."))
		return;
	}
	
	// TODO: handle more then 1 bone
	FNCellInputDataFilter PreFilter;
	PreFilter.SocketSize = BoneInputData[0].SocketSize;
	PreFilter.SourceQuat = FQuat(BoneInputData[0].WorldRotation);
	PreFilter.bRequireStart = true;
	
	FNWeightedIntegerArray PreIndices;
	TMap<int32, TArray<int32>> ValidJunctions;
	
	FilterCellInputData(PreFilter, PreIndices, ValidJunctions);

	if (PreIndices.Count() == 0)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as no starting junctions are sized to the provided UNBoneComponent."))
		return;
	}
	
	
	// We got to the end so we have validated things so far at this point.
	bIsValid = true;
}

FNOrganGraphBuilderContext::~FNOrganGraphBuilderContext()
{
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory	
	if (CellGraph != nullptr)
	{
		delete CellGraph.Release();
		CellGraph = nullptr;
	}
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory
}

bool FNOrganGraphBuilderContext::CheckGraph() const
{
	// We're going to look over all the nodes
	int CellNodeCount = 0;
	for (const auto Pair : CellGraph->GetNodes())
	{
		if (Pair->GetNodeType() == ENProcGenGraphNodeType::Cell)
		{
			CellNodeCount++;
		}
	}
	
	// Enforce check for the minimum amount of cells wanted
	if (MinimumCellCount > 0 && CellNodeCount < MinimumCellCount)
	{
		return false;
	}
	
	// Enforce check for maximum amount of cells wanted
	if (MaximumCellCount > 0 && CellNodeCount > MaximumCellCount)
	{
		return false;
	}

	return true;
}

bool FNOrganGraphBuilderContext::ValidateGraph()
{
	bSuccessful = CheckGraph();
	return bSuccessful;
}

void FNOrganGraphBuilderContext::FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices)
{
	CellIndices.Empty();
	JunctionIndices.Empty();
	
	for (int i = 0; i < CellInputData.Num(); i++)
	{
		const FNCellInputData* CellData = &CellInputData[i];
		
		
		// Early out on some simple filters
		if (!CellData->IsValidSelection(Filter.SocketSize)) continue;
		if (Filter.bRequireStart && !CellData->bCanBeStartNode) continue;
		if (Filter.bRequireEnd && !CellData->bCanBeEndNode) continue;
		
		if (Filter.SourceCellInputData != nullptr && Filter.SourceCellInputData == CellData) continue;
		
		
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

bool FNOrganGraphBuilderContext::ResetForRetry(FNOrganGraphBuilderAnalytics& Analytics)
{
	// Reset counters
	for (int i = 0; i < CellInputData.Num(); i++)
	{
		CellInputData[i].UsedCount = 0;
	}
		
	// Update analytics
	Analytics.Retry();
		
	// Clear Graph
	if (CellGraph != nullptr)
	{
		delete CellGraph.Release();
		CellGraph = nullptr;
	}
	
	RetryCount++;
	
	if (RetryCount > MaximumRetryCount)
	{
		return false;
	}
	return true;
}
