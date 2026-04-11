// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTaskContext.h"

#include "NProcGenMinimal.h"
#include "ParticleHelper.h"
#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Generation/NProcGenOperationContext.h"
#include "Organ/NOrganComponent.h"


FNOrganGeneratorTaskContext::FNOrganGeneratorTaskContext(const FNProcGenOperationOrganContext* GeneratorContextMap, const uint64 TaskSeed)
	: Seed(TaskSeed)
{
	// This is our last chance to read anything off the main-thread
	//TODO: There is a Seed on the component? What do we do here with it?
	
	// We are going to establish some base understanding of the space, specifically its world origin as well as the bounds.
	if (GeneratorContextMap->SourceComponent->IsVolumeBased())
	{
		const AVolume* Volume = GeneratorContextMap->SourceComponent->GetVolume();
		
		Bounds = Volume->GetBounds();
		Origin = Volume->GetActorLocation();
	}
	else
	{
		// TODO: Handle non-volume based generation
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
	
	// Check that we have something of size to socket
	// TODO: handle more then 1 bone
	FNWeightedIntegerArray WeightedStartIndices = GenerateWeightedStartCellIndices(BoneInputData[0].SocketSize);
	if (WeightedStartIndices.Count() == 0)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to validate FNOrganGeneratorTaskContext as no starting junctions are sized to the provided UNBoneComponent."))
		return;
	}
	
	
	// We got to the end so we have validated things so far at this point.
	bIsValid = true;
}

FNOrganGeneratorTaskContext::~FNOrganGeneratorTaskContext()
{
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory	
	if (CellGraph != nullptr)
	{
		delete CellGraph.Release();
		CellGraph = nullptr;
	}
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory
}


FNWeightedIntegerArray FNOrganGeneratorTaskContext::GenerateWeightedCellInputIndices()
{
	FNWeightedIntegerArray WeightedIntegers;
	
	for (int i = 0; i < CellInputData.Num(); i++)
	{
		const FNCellInputData* CellData = &CellInputData[i];
		if (CellData->MaximumCount == 0) continue;
		
		WeightedIntegers.Add(i, CellData->Weighting);
	}

	return MoveTemp(WeightedIntegers);
}

FNWeightedIntegerArray FNOrganGeneratorTaskContext::GenerateWeightedStartCellIndices(FIntVector2 RequestedSocketSize)
{
	FNWeightedIntegerArray WeightedIntegers;
	
	for (int i = 0; i < CellInputData.Num(); i++)
	{
		const FNCellInputData* CellData = &CellInputData[i];
		if (CellData->MaximumCount == 0 || !CellData->bCanBeStartNode) continue;
		
		// Parse Junctions
		if (CellData->Junctions.Num() == 0) continue;
		for (auto Pair : CellData->Junctions)
		{
			if (Pair.Value.SocketSize == RequestedSocketSize)
			{
				WeightedIntegers.Add(i, CellData->Weighting);
				break;
			}
		}
	}

	return MoveTemp(WeightedIntegers);
}

FNWeightedIntegerArray FNOrganGeneratorTaskContext::GenerateWeightedCellInputIndices(FIntVector2 RequestedSocketSize)
{
	FNWeightedIntegerArray WeightedIntegers;
	
	for (int i = 0; i < CellInputData.Num(); i++)
	{
		const FNCellInputData* CellData = &CellInputData[i];
		if (CellData->MaximumCount == 0) continue;
		
		// Parse Junctions
		if (CellData->Junctions.Num() == 0) continue;
		for (auto Pair : CellData->Junctions)
		{
			if (Pair.Value.SocketSize == RequestedSocketSize)
			{
				WeightedIntegers.Add(i, CellData->Weighting);
				break;
			}
		}
	}

	return MoveTemp(WeightedIntegers);
}
