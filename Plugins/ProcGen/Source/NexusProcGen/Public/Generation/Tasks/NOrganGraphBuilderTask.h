// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCollectPassContext.h"
#include "Generation/Tasks/NOrganGraphBuilderContext.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Async/TaskGraphInterfaces.h"

struct FNOrganGraphBuilderTask
{
	explicit FNOrganGraphBuilderTask(const TSharedPtr<FNOrganGraphBuilderContext>& ContextPtr, 
		const TSharedPtr<FNCollectPassContext>& PassContextPtr);
    
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenGraphBuilderTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	
	TSharedRef<FNOrganGraphBuilderContext> ContextPtr;
	TSharedRef<FNCollectPassContext> PassContextPtr;
	
	mutable FNOrganGraphBuilderAnalytics Analytics;
	
	void StartGraph(FNMersenneTwister& Random) const;
	
	TArray<FNProcGenGraphCellNode*> CheckNodeBounds(FNProcGenGraphCellNode* NewNode) const;
	TArray<FNProcGenGraphCellNode*> CheckNodeHull(FNProcGenGraphCellNode* NewNode) const;
	
	TArray<FNProcGenGraphNode*> ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const;
	TArray<FNProcGenGraphNode*> ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const;
};
