// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Async/TaskGraphInterfaces.h"

class UNOrganGenerator;
class FNOrganGeneratorComponentMap;

struct FNOrganGeneratorFinalizeTask
{
public:
	
	explicit FNOrganGeneratorFinalizeTask(UNOrganGenerator* OrganGenerator);
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNOrganGraphFinalizeTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);
private:
	UNOrganGenerator* Generator;
};
