// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NProcGenTaskAnalytics.h"
#include "Generation/Contexts/NVirtualWorldContext.h"

class FNProcessVirtualWorldTask
{
public:
	explicit FNProcessVirtualWorldTask(const TSharedPtr<FNVirtualWorldContext>& VirtualWorldContextPtr 
		N_PROC_GEN_ANALYTICS_CONSTRUCTOR) 
	: VirtualWorldContextPtr(VirtualWorldContextPtr.ToSharedRef()) N_PROC_GEN_ANALYTICS_INITIALIZER { }

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcessVirtualWorldTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	TSharedRef<FNVirtualWorldContext> VirtualWorldContextPtr;
	N_PROC_GEN_ANALYTICS_SHARED_REF
};
