// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcGenCreateProxiesTask.h"
#include "Generation/Graph/NProcGenGraph.h"
#include "Generation/NProcGenOperationSharedContext.h"

FNProcGenCreateProxiesTask::FNProcGenCreateProxiesTask(TUniquePtr<FNProcGenGraph> CellGraph, TSharedRef<FNProcGenOperationSharedContext> SharedContext)
{
}

FNProcGenCreateProxiesTask::~FNProcGenCreateProxiesTask()
{
}

void FNProcGenCreateProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
}
