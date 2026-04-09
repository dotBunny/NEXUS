// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

#include "NBoxPicker.h"
#include "NProcGenMinimal.h"
#include "Generation/NCellOutputData.h"
#include "Math/NMersenneTwister.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
	: Context(ContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{
}

// TODO: Spin up multiple of these tasks? with different sub-seeds? Maybe we make a seperate task under this
void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Create our deterministic random for the task
	FNMersenneTwister Random(Context->Seed);
	
	// Create our weighted array fresh.
	FNWeightedIntegerArray WeightedCellInputIndices = Context->GenerateWeightedCellInputIndices();
	
	// TODO: Move validation outside of task
	if (Context->BoneInputData.Num() == 0)
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to execute FNOrganGeneratorTask as no bones were provided for generation."))
		return;
	}
	
	// TODO: We haven't resolved yet how we might join multiple generation points yet so we are just going to use the first bone.
	FNBoneInputData& BoneData = Context->BoneInputData[0];
	
	// TODO: Move validation outside of task
	if (!WeightedCellInputIndices.HasData())
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to execute FNOrganGeneratorTask as no weighted indices were created from the input cell data."))
		return;
	}
	
	
	// IMPLEMENT
	// - use bones position/rotation/socket size to find a matching cell that allows for first
	// - loop through junctions on cell
	//		- decide if fill or not
	//		- find cell to attach
	// - loop throw all newly added cells and their junctions 
	//      - decide if fill or not
	//      - find cell to attach
	
	
	
		
	// REMINDERS
	// - Need a quick way to make sure that a spawned cell is INSIDE the volume
	
	
	
	
	// TEMP-START: This was just used to test FNOrganGeneratorFinalizeTask
	TArray<FVector> Locations;
	FNBoxPickerParams Params;
	Params.Count = Context->CellInputData.Num();
	Params.Origin = Context->Bounds.Origin;
	Params.MaximumBox = FBox(-Context->Bounds.BoxExtent, Context->Bounds.BoxExtent);
	FNBoxPicker::Twisted(Locations, Random, Params);
	for (int i = 0; i < Context->CellInputData.Num(); i++)
	{
		FNCellOutputData CellOutput;
		CellOutput.WorldPosition = Locations[i];
		CellOutput.Template = Context->CellInputData[i].Template;
		Context->CellOutputData.Add(CellOutput);
	}
	// TEMP-END: This was just used to test FNOrganGeneratorFinalizeTask

	Context->bSuccessful = true;
}
