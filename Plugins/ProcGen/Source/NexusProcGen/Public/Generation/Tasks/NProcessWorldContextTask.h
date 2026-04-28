// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NWorldContext.h"

class FNProcessWorldContextTask
{
public:
	explicit FNProcessWorldContextTask(const TSharedPtr<FNWorldContext>& WorldContextPtr, const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr) 
	: WorldContextPtr(WorldContextPtr.ToSharedRef()), AnalyticsPtr(AnalyticsPtr.ToSharedRef())
	{
		
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcessWorldContextTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	TSharedRef<FNWorldContext> WorldContextPtr;
	TSharedRef<FNProcGenTaskAnalytics> AnalyticsPtr;
};
