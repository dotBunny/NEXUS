// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

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
	
	
	// We noww have to figure out placement of the objects in the space of the organ
	// We also need to somehow create an extensible thing here?
	
	// This should only figure out placement of items in side of the graph
	// We need to look at tissue ? get items / etc
	
	
	
	// TODO: lets just make it do something
	FPlatformProcess::Sleep(Random.RandRange(0.25f, 1.5f)); 
}
