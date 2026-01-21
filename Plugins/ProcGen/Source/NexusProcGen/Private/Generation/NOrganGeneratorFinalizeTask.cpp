// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorFinalizeTask.h"

#include "NProcGenOperation.h"


FNOrganGeneratorFinalizeTask::FNOrganGeneratorFinalizeTask(
	UNProcGenOperation* TargetOperation, const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr)
		: Operation(TargetOperation), Context(ContextPtr.ToSharedRef())
{

}

void FNOrganGeneratorFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
//	FPlatformProcess::Sleep(5.0f); 
// Dont sleep on main thread K thnx	
	
//	Operation->FinishOrgan();
}
