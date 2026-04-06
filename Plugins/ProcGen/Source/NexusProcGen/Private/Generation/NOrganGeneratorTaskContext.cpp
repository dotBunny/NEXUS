// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTaskContext.h"

#include "Cell/NCell.h"
#include "Cell/NTissue.h"
#include "Generation/NProcGenOperationContext.h"
#include "Organ/NOrganComponent.h"


FNOrganGeneratorTaskContext::FNOrganGeneratorTaskContext(const FNOrganGenerationContext* GeneratorContextMap, const uint64 TaskSeed)
	: Seed(TaskSeed)
{
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
	for (const auto& Cell : TissueMap)
	{
		FNCellInputData SafeDetails;
		
		// TODO: We could implement some checks on the UNCell about cross referencing and what happens?
		SafeDetails.MinimumCount = Cell.Value.MinimumCount;
		SafeDetails.MaximumCount = Cell.Value.MaximumCount;
		SafeDetails.Weighting = Cell.Value.Weighting;
		
		// We won't touch this till later
		SafeDetails.Template = Cell.Key->World;
		
		Cell.Key->Root.CopyTo(SafeDetails.CellDetails);
		for (const TPair<int32, FNCellJunctionDetails>& Junction :  Cell.Key->Junctions)
		{
			SafeDetails.Junctions.Add(Junction.Key, Junction.Value);
		}
	}
}
