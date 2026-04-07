// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenOperationFinalizeTask.h"
#include "NProcGenOperation.h"


FNProcGenOperationFinalizeTask::FNProcGenOperationFinalizeTask(UNProcGenOperation* TargetOperation, 
	const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr)
	: Operation(TargetOperation), SharedContext(SharedContextPtr.ToSharedRef())
{
	
}

void FNProcGenOperationFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// SPAWN STUFF from context?
	// This is run on main thread so we should be able to spawn
	
	// We need to keep a record of the created proxies?
	
	// TODO: We need to save these created context stuff from shared somewhere?
	
	Operation->FinishBuild(SharedContext);
}
