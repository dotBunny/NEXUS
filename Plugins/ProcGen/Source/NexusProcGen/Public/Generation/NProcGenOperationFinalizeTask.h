// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenOperationSharedContext.h"
#include "Async/TaskGraphInterfaces.h"

class UNProcGenOperation;

struct FNProcGenOperationFinalizeTask
{
public:
	
	explicit FNProcGenOperationFinalizeTask(UNProcGenOperation* TargetOperation, const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr);
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenOperationFinalizeTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	UNProcGenOperation* Operation;
	TSharedRef<FNProcGenOperationSharedContext> SharedContext;
};
