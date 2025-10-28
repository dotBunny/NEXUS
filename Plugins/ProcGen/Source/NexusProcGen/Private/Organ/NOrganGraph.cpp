// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGraph.h"

#include "Organ/NOrganContext.h"
#include "Organ/NOrganGraphFinalizeTask.h"
#include "Organ/NOrganGraphTask.h"

FNOrganGraph::FNOrganGraph(FNOrganContext* Context)
{
	bTasksUnlocked = false;
	
	// Assign the base level context to the graph
	RootContext = Context;
	
	// Build out processing tasks
	for (auto Pass : RootContext->GenerationOrder)
	{
		FGraphEventArray Tasks;
		for (const auto Component : Pass)
		{
			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganTask = TGraphTask<FNOrganGraphTask>::CreateTask(
				(PassTasks.Num() > 0) ? &PassTasks.Last() : nullptr, 
				ENamedThreads::AnyNormalThreadNormalTask) // ENamedThreads::GameThread
				.ConstructAndHold(RootContext->Components.Find(Component)); // ConstructAndDispatchWhenReady
			
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
	RootContext = nullptr;
	bTasksUnlocked = false;
}
