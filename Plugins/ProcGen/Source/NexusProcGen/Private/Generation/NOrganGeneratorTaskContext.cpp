// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTaskContext.h"

#include "Cell/NCell.h"
#include "Cell/NTissue.h"
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
	CellInputData.Reserve(TissueMap.Num());
	for (const auto& Cell : TissueMap)
	{
		FNCellInputData CellDetails;
		
		// TODO: We could implement some checks on the UNCell about cross referencing and what happens?
		CellDetails.MinimumCount = Cell.Value.MinimumCount;
		CellDetails.MaximumCount = Cell.Value.MaximumCount;
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
		
		BoneDetails.MinimumPoint = Bone->MinimumPoint;
		BoneDetails.MaximumPoint = Bone->MaximumPoint;		
		
		BoneDetails.SocketSize = Bone->SourceComponent->SocketSize;
		
		BoneInputData.Add(BoneDetails);
	}
}
