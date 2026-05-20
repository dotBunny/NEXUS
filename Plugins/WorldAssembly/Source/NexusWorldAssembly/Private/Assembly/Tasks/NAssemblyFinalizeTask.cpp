// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NAssemblyFinalizeTask.h"
#include "Assembly/NAssemblyOperation.h"


FNAssemblyFinalizeTask::FNAssemblyFinalizeTask(UNAssemblyOperation* TargetOperation, 
                                                         const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: Operation(TargetOperation), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER
{
	
}

void FNAssemblyFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
#if !UE_BUILD_SHIPPING	
	N_ASSEMBLY_ANALYTICS_MEMBER_PTR->AddToReport(Operation->GetReport());
	Operation->OutputReportToFile();
#endif // !UE_BUILD_SHIPPING
	
	// Send the finalized shared data back to the operation for doings
	// This usually means recording the spawned things somewhere, but also could be useful for triggering events.
	Operation->FinishBuild(TaskGraphContextPtr);
	
	// The operation at this point has been flagged as garbage, don't use it.
}
