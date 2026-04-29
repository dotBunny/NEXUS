// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcGenFinalizeTask.h"
#include "NProcGenOperation.h"


FNProcGenFinalizeTask::FNProcGenFinalizeTask(UNProcGenOperation* TargetOperation, 
	const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr N_PROCEDURAL_GENERATION_ANALYTICS_CONSTRUCTOR)
	: Operation(TargetOperation), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_PROCEDURAL_GENERATION_ANALYTICS_INITIALIZER
{
	
}

void FNProcGenFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_PROCEDURAL_GENERATION_ANALYTICS(ProcGenFinalizeStart)

	// Send the finalized shared data back to the operation for doings
	// This usually means recording the spawned things somewhere, but also could be useful for triggering events.
	Operation->FinishBuild(TaskGraphContextPtr);

	N_PROCEDURAL_GENERATION_ANALYTICS(ProcGenFinalizeFinish)

#if !UE_BUILD_SHIPPING	
	// Log our analytics at this point
	UE_LOG(LogNexusProcGen, Log, TEXT("%s"), *N_PROCEDURAL_GENERATION_ANALYTICS_MEMBER_PTR->GetTimespanReport());
#endif // !UE_BUILD_SHIPPING
}
