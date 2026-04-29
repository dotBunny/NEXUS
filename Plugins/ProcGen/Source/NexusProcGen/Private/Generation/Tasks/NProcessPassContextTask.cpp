// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcessPassContextTask.h"

FNProcessPassContextTask::FNProcessPassContextTask(const TSharedPtr<FNPassContext>& PassContextPtr, 
                                                   const TSharedPtr<FNWorldContext>& WorldContextPtr, 
                                                   const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr, 
                                                   const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr) 
:	PassContextPtr(PassContextPtr.ToSharedRef()), WorldContextPtr(WorldContextPtr.ToSharedRef()), 
	TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()), AnalyticsPtr(AnalyticsPtr.ToSharedRef())
{
	// Stub our analytics around this
#if !UE_BUILD_SHIPPING	
	AnalyticsIndex = AnalyticsPtr->CollectGenerationPassesCreate();
#endif // !UE_BUILD_SHIPPING
}

void FNProcessPassContextTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->CollectGenerationPassesStart(AnalyticsIndex);
#endif // !UE_BUILD_SHIPPING
	
	// TODO: Copy things from pass context to shared context
	
	// TODO: Move collision meshes to world context
	
	for (TUniquePtr<FNProcGenGraph>& Graph : PassContextPtr->Graphs)
	{
		// TODO: is there any bads about using the shared ptr here .. other threads? locks?
		
		TaskGraphContextPtr->TakeGraph(MoveTemp(Graph));
	}
	PassContextPtr->Graphs.Reset();
	
#if !UE_BUILD_SHIPPING	
	AnalyticsPtr->CollectGenerationPassesFinish(AnalyticsIndex);
#endif // !UE_BUILD_SHIPPING	
}
