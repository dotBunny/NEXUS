// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NOrganGraphTaskContext.h"
#include "Async/TaskGraphInterfaces.h"

struct FNOrganGraphTask
{
	explicit FNOrganGraphTask(FNOrganGraphTaskContext* ContextPtr);
    
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNOrganGraphTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	FNOrganGraphTaskContext* Context;
};
