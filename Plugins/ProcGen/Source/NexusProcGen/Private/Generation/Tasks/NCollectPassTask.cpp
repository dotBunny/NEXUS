// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NCollectPassTask.h"

FNCollectPassTask::FNCollectPassTask(const TSharedPtr<FNCollectPassContext>& PassContextPtr, const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr) 
: PassContextPtr(PassContextPtr.ToSharedRef()), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef())
{
}

void FNCollectPassTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// TODO: Copy things from pass context to shared context
	
	for (TUniquePtr<FNProcGenGraph>& Graph : PassContextPtr->Graphs)
	{
		// TODO: is there any bads about using the shared ptr here .. other threads? locks?
		
		TaskGraphContextPtr->TakeGraph(MoveTemp(Graph));
	}
	PassContextPtr->Graphs.Reset();
}
