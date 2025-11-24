// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGeneratorFinalizeTask.h"

#include "Organ/NOrganGenerator.h"


FNOrganGeneratorFinalizeTask::FNOrganGeneratorFinalizeTask(UNOrganGenerator* OrganGenerator)
{
	Generator = OrganGenerator;
}

void FNOrganGeneratorFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	FPlatformProcess::Sleep(5.0f); 
	
	
	Generator->FinishBuild();
}
