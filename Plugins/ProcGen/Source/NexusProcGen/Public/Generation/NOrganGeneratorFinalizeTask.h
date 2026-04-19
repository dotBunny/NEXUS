// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NOrganGeneratorPassContext.h"
#include "Generation/Tasks/NProcGenGraphBuilderContext.h"
#include "NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"

class UNProcGenOperation;
class FNOrganGeneratorComponentMap;

// Task to create objects in world?
struct FNOrganGeneratorFinalizeTask
{
public:
	explicit FNOrganGeneratorFinalizeTask(UNProcGenOperation* TargetOperation, 
		const TSharedPtr<FNProcGenGraphBuilderContext>& ContextPtr, 
		const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr, 
		const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr);
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNOrganGraphFinalizeTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	UNProcGenOperation* Operation;
	TSharedRef<FNProcGenGraphBuilderContext> Context;
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContextPtr;
};
