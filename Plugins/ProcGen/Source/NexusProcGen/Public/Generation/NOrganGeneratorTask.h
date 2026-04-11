// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NOrganGeneratorPassContext.h"
#include "NOrganGeneratorTaskContext.h"
#include "NProcGenOperationSharedContext.h"
#include "Async/TaskGraphInterfaces.h"

struct FNOrganGeneratorTask
{
	explicit FNOrganGeneratorTask(const TSharedPtr<FNOrganGeneratorTaskContext>& ContextPtr, 
		const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
		const TSharedPtr<FNProcGenOperationSharedContext>& SharedContextPtr);
    
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNOrganGraphTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	TSharedRef<FNOrganGeneratorTaskContext> Context;
	TSharedRef<FNOrganGeneratorPassContext> PassContext;
	TSharedRef<FNProcGenOperationSharedContext> SharedContext;
};
