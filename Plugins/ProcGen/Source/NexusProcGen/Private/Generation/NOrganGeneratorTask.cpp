// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

#include "Math/NMersenneTwister.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(FNOrganGeneratorTaskContext* ContextPtr)
{
	Context = ContextPtr;
}

void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Create our deterministic random for the task
	FNMersenneTwister Random(Context->Seed);
	
	
	
	
	// TODO: lets just make it do something
	FPlatformProcess::Sleep(Random.RandRange(0.25f, 1.5f)); 
}
