// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGeneratorTasks.h"

#include "Organ/NOrganGeneratorContext.h"
#include "Organ/NOrganGeneratorFinalizeTask.h"
#include "Organ/NOrganGeneratorTask.h"

FNOrganGeneratorTasks::FNOrganGeneratorTasks(UNOrganGenerator* Generator, FNOrganGeneratorContext* Context)
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
			FNOrganGeneratorTaskContext* ContextPtr = new FNOrganGeneratorTaskContext(ContextMap);
			
			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganTask = TGraphTask<FNOrganGeneratorTask>::CreateTask(
				(PassTasks.Num() > 0) ? &PassTasks.Last() : nullptr, 
				ENamedThreads::AnyNormalThreadNormalTask) // ENamedThreads::GameThread
				.ConstructAndHold(ContextPtr); // ConstructAndDispatchWhenReady
			
			Tasks.Add(OrganTask);
		}

		/// Ensure we track a passes tasks
		PassTasks.Add(Tasks);
	};
	
	// Create our finalizer task on main thread
	FinalizeTask = TGraphTask<FNOrganGeneratorFinalizeTask>::CreateTask(&PassTasks.Last(), ENamedThreads::GameThread).ConstructAndHold(Generator);
}

void FNOrganGeneratorTasks::UnlockTasks()
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

void FNOrganGeneratorTasks::WaitForTasks()
{
	if (!bTasksUnlocked)
	{
		UnlockTasks();
	}

	FTaskGraphInterface::Get().WaitUntilTaskCompletes(FinalizeTask);
}

void FNOrganGeneratorTasks::Reset()
{
	bTasksUnlocked = false;
}
