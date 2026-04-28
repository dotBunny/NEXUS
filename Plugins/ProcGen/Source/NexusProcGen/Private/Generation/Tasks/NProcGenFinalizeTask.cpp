// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcGenFinalizeTask.h"
#include "NProcGenOperation.h"


FNProcGenFinalizeTask::FNProcGenFinalizeTask(UNProcGenOperation* TargetOperation, 
	const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr, const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr)
	: Operation(TargetOperation), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), AnalyticsPtr(AnalyticsPtr.ToSharedRef())
{
	
}

void FNProcGenFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->ProcGenFinalizeStart();
#endif // !UE_BUILD_SHIPPING		
	// Send the finalized shared data back to the operation for doings
	// This usually means recording the spawned things somewhere, but also could be useful for triggering events.
	Operation->FinishBuild(TaskGraphContextPtr);

#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->ProcGenFinalizeFinish();
	
	// Log our analytics at this point
	UE_LOG(LogNexusProcGen, Log, TEXT("%s"), *AnalyticsPtr->GetTimespanReport());
#endif // !UE_BUILD_SHIPPING
}
