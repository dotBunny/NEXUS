// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenOperationFinalizeTask.h"

#include "NProcGenOperation.h"


FNProcGenOperationFinalizeTask::FNProcGenOperationFinalizeTask(UNProcGenOperation* TargetOperation)
	: Operation(TargetOperation)
{
	
}

void FNProcGenOperationFinalizeTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// SPAWN STUFF from context?
	// This is run on main thread so we should be able to spawn
	
	Operation->FinishBuild();
}
