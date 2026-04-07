// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

#include "NBoxPicker.h"
#include "Generation/NCellOutputData.h"
#include "Math/NMersenneTwister.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr,
	const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
	: Context(ContextPtr.ToSharedRef()), SharedContext(SharedContextPtr.ToSharedRef())
{
}

void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Create our deterministic random for the task
	FNMersenneTwister Random(Context->Seed);
	
	// TODO: Spin up multiple tasks? with different subseeds? t0 do the builds?
	
	// TEMP: Were just going to pick some random spots inside of the bounds to put things for now
	
	TArray<FVector> Locations;
	FNBoxPickerParams Params;
	Params.Count = Context->CellInputData.Num();
	Params.Origin = Context->Bounds.Origin;
	
	// We'd want to minimize this too
	Params.MaximumBox = FBox(-Context->Bounds.BoxExtent, Context->Bounds.BoxExtent);
	
	
	// Pick the origin points (were gonna overlap for now)
	FNBoxPicker::Twisted(Locations, Random, Params);
	
	for (int i = 0; i < Context->CellInputData.Num(); i++)
	{
		FNCellOutputData CellOutput;
		
		CellOutput.WorldPosition = Locations[i];
		CellOutput.Template = Context->CellInputData[i].Template;
	
		Context->CellOutputData.Add(CellOutput);
	}
	

	
	//Context->CellOutputData.Add()
	
	
	
	
	// We noww have to figure out placement of the objects in the space of the organ
	// We also need to somehow create an extensible thing here?
	
	// This should only figure out placement of items in side of the graph
	// We need to look at tissue ? get items / etc

	// Use hulls
	//	Context->CellInputData[0].CellDetails.Hull
	

}
