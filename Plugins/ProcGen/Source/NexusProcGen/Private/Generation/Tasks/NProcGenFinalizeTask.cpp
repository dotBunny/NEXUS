// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcGenFinalizeTask.h"
#include "NProcGenOperation.h"


FNProcGenFinalizeTask::FNProcGenFinalizeTask(UNProcGenOperation* TargetOperation, 
	const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr)
	: Operation(TargetOperation), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef())
{
	
}

void FNProcGenFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Send the finalized shared data back to the operation for doings
	// This usually means recording the spawned things somewhere, but also could be useful for triggering events.
	Operation->FinishBuild(TaskGraphContextPtr);
}
