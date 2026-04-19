// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/NOrganGeneratorPassContext.h"
#include "Generation/Tasks/NProcGenGraphBuilderContext.h"
#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Generation/NProcGenTaskGraphContext.h"
#include "Async/TaskGraphInterfaces.h"

struct FNProcGenGraphBuilderTask
{
	explicit FNProcGenGraphBuilderTask(const TSharedPtr<FNProcGenGraphBuilderContext>& ContextPtr, 
		const TSharedPtr<FNOrganGeneratorPassContext>& PassContextPtr,
		const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr);
    
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcGenGraphBuilderTask, STATGROUP_TaskGraphTasks); }
    
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyBackgroundThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:

	void StartGraph(FNMersenneTwister& Random) const;
	
	TArray<FNProcGenGraphCellNode*> CheckNodeBounds(FNProcGenGraphCellNode* NewNode) const;
	TArray<FNProcGenGraphCellNode*> CheckNodeHull(FNProcGenGraphCellNode* NewNode) const;
	
	TArray<FNProcGenGraphNode*> ProcessNode(FNMersenneTwister& Random, FNProcGenGraphNode* SourceNode) const;
	TArray<FNProcGenGraphNode*> ProcessCellNode(FNMersenneTwister& Random, FNProcGenGraphCellNode* SourceCellNode) const;
	
	TSharedRef<FNProcGenGraphBuilderContext> Context;
	TSharedRef<FNOrganGeneratorPassContext> PassContext;
	TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext;
};
