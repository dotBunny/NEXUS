// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorFinalizeUnsafeTask.h"

#include "NProcGenOperation.h"


FNOrganGeneratorFinalizeUnsafeTask::FNOrganGeneratorFinalizeUnsafeTask(UNProcGenOperation* OrganGenerator)
{
	Operation = OrganGenerator;
}

void FNOrganGeneratorFinalizeUnsafeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	FPlatformProcess::Sleep(5.0f); 
	
	
	Operation->FinishBuild();
}
