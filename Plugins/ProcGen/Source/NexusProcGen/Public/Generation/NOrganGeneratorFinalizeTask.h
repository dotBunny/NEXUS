// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NOrganGeneratorTaskContext.h"
#include "Async/TaskGraphInterfaces.h"

class UNProcGenOperation;
class FNOrganGeneratorComponentMap;

struct FNOrganGeneratorFinalizeTask
{
public:
	explicit FNOrganGeneratorFinalizeTask(UNProcGenOperation* TargetOperation, FNOrganGeneratorTaskContext* ContextPtr);
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNOrganGraphFinalizeTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	UNProcGenOperation* Operation;
	FNOrganGeneratorTaskContext* Context;
};
