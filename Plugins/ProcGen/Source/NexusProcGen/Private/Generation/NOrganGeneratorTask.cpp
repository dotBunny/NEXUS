// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTask.h"

FNOrganGeneratorTask::FNOrganGeneratorTask(FNOrganGeneratorTaskContext* ContextPtr)
{
	Context = ContextPtr;
}

void FNOrganGeneratorTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// TODO: lets just make it do something
	FPlatformProcess::Sleep(5.0f); 
}
