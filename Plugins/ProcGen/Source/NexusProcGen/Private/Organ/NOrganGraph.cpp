// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGraph.h"

#include "Organ/NOrganGeneratorContext.h"
#include "Organ/NOrganGraphFinalizeTask.h"
#include "Organ/NOrganGraphTask.h"

FNOrganGraph::FNOrganGraph(FNOrganGeneratorContext* Context)
{
	bTasksUnlocked = false;
	
	
	// TODO: Generate NOrganGraphContext?
	
	
	// Build out processing tasks
	for (auto Pass : Context->GenerationOrder)
	{
		FGraphEventArray Tasks;
		for (const auto Component : Pass)
		{
			// Create component context
			FNOrganGeneratorContextMap* ContextMap = Context->Components.Find(Component);
			FNOrganGraphTaskContext* ContextPtr = new FNOrganGraphTaskContext(ContextMap);
			
			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganTask = TGraphTask<FNOrganGraphTask>::CreateTask(
				(PassTasks.Num() > 0) ? &PassTasks.Last() : nullptr, 
				ENamedThreads::AnyNormalThreadNormalTask) // ENamedThreads::GameThread
				.ConstructAndHold(ContextPtr); // ConstructAndDispatchWhenReady
			
			Tasks.Add(OrganTask);
		}

		/// Ensure we track a passes tasks
		PassTasks.Add(Tasks);
	};
	
	// Create our finalizer task
	FinalizeTask = TGraphTask<FNOrganGraphFinalizeTask>::CreateTask(&PassTasks.Last(), ENamedThreads::AnyNormalThreadNormalTask).ConstructAndHold();
}

void FNOrganGraph::UnlockTasks()
{
	// Start running the tasks
	for (const auto Tasks : PassTasks)
	{
		for (const auto Task : Tasks)
		{
			Task->Unlock();
		}
	}
	FinalizeTask->Unlock();
	
	bTasksUnlocked = true;
}

void FNOrganGraph::WaitForTasks()
{
	if (!bTasksUnlocked)
	{
		UnlockTasks();
	}

	FTaskGraphInterface::Get().WaitUntilTaskCompletes(FinalizeTask);
}

void FNOrganGraph::Reset()
{
	bTasksUnlocked = false;
}
