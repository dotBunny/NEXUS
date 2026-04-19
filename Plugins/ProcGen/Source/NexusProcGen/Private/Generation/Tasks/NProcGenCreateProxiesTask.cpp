// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NProcGenCreateProxiesTask.h"
#include "Generation/NProcGenTaskGraphContext.h"

FNProcGenCreateProxiesTask::FNProcGenCreateProxiesTask(const TSharedPtr<FNProcGenTaskGraphContext>& TaskGraphContextPtr)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef())
{
}

FNProcGenCreateProxiesTask::~FNProcGenCreateProxiesTask()
{
}

void FNProcGenCreateProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
}
