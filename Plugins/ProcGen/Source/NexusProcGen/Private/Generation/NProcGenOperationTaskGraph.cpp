// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenOperationTaskGraph.h"

#include "NProcGenMinimal.h"

#include "Generation/NProcGenOperationContext.h"
#include "Generation/NOrganGeneratorFinalizeTask.h"
#include "Generation/NOrganGeneratorTask.h"
#include "Generation/NProcGenOperationFinalizeTask.h"

#include "Math/NMersenneTwister.h"
#include "Math/NSeedGenerator.h"

FNProcGenOperationTaskGraph::FNProcGenOperationTaskGraph(UNProcGenOperation* Generator, FNProcGenOperationContext* Context)
{
	// Convert our friendly seed to something more appropriate
	const uint64 BaseSeed = FNSeedGenerator::SeedFromFriendlySeed(Context->FriendlySeed);
	UE_LOG(LogNexusProcGen, Log, TEXT("Converted friendly seed(%s) to uint64 seed(%llu)"), *Context->FriendlySeed, BaseSeed);
	FNMersenneTwister BaseGenerator(BaseSeed);
	uint32 SubTaskIndex = 0;
	// Build out the organ generation tasks, with finalizers
	for (auto Pass : Context->GenerationOrder)
	{
		FGraphEventArray Tasks;
		for (const auto Component : Pass)
		{
			// Create component context
			FNOrganGenerationContext* ContextMap = Context->Components.Find(Component);
			
			// Create individual organ generator context object
			TSharedPtr<FNOrganGeneratorTaskContext> ContextPtr = MakeShared<FNOrganGeneratorTaskContext>(ContextMap, BaseGenerator.UnsignedInteger64());
			
			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganTask = TGraphTask<FNOrganGeneratorTask>::CreateTask(
				(PassTasks.Num() > 0) ? &PassTasks.Last() : nullptr, 
				ENamedThreads::AnyNormalThreadNormalTask) // ENamedThreads::GameThread
				.ConstructAndHold(ContextPtr); // ConstructAndDispatchWhenReady
			Tasks.Add(OrganTask);
			
			// Create a task to finalize the previous organ task on the main thread
			FGraphEventArray SubTasks;
			SubTasks.Add(OrganTask);
			FGraphEventRef OrganFinalizeTask = TGraphTask<FNOrganGeneratorFinalizeTask>::CreateTask(&SubTasks, 
				ENamedThreads::GameThread)
				.ConstructAndHold(Generator, ContextPtr);
			Tasks.Add(OrganFinalizeTask);
		}
		
		SubTaskIndex++;

		/// Ensure we track a passes tasks
		PassTasks.Add(Tasks);
	};
	
	// Create our finalizer task on the main thread
	FinalizeTask = TGraphTask<FNProcGenOperationFinalizeTask>::CreateTask(&PassTasks.Last(), ENamedThreads::GameThread).ConstructAndHold(Generator);
}

void FNProcGenOperationTaskGraph::UnlockTasks()
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

void FNProcGenOperationTaskGraph::WaitForTasks()
{
	if (!bTasksUnlocked)
	{
		UnlockTasks();
	}

	FTaskGraphInterface::Get().WaitUntilTaskCompletes(FinalizeTask);
}

void FNProcGenOperationTaskGraph::ResetGraph()
{
	bTasksUnlocked = false;
}

int FNProcGenOperationTaskGraph::GetTotalPasses() const
{
	return PassTasks.Num();
}

int FNProcGenOperationTaskGraph::GetCompletedPasses()
{
	int CompletedPasses = 0;
	for (auto Pass : PassTasks)
	{
		bool bPassComplete = true;
		for (const FGraphEventRef& Task : Pass)
		{
			if (!Task->IsComplete())
			{
				bPassComplete = false;
				break;
			}
		}
		if (!bPassComplete)
		{
			break;
		}
		CompletedPasses++;
	}
	
	return CompletedPasses;
}

FIntVector2 FNProcGenOperationTaskGraph::GetTaskStatus() const
{
	int TotalTasks = 0;
	int CompletedTasks = 0;
	for (auto Pass : PassTasks)
	{
		for (const FGraphEventRef& Task : Pass)
		{
			if (Task->IsComplete())
			{
				CompletedTasks++;
			}
			TotalTasks++;
		}
	}
	return FIntVector2(CompletedTasks, TotalTasks);
}