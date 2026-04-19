// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenTaskGraph.h"

#include "NProcGenMinimal.h"

#include "Generation/NProcGenOperationContext.h"
#include "Generation/NOrganGeneratorFinalizeTask.h"
#include "Generation/NOrganGeneratorPassContext.h"
#include "Generation/Tasks/NProcGenGraphBuilderTask.h"
#include "Generation/NProcGenOperationFinalizeTask.h"
#include "Generation/NProcGenTaskGraphContext.h"

#include "Math/NMersenneTwister.h"
#include "Math/NSeedGenerator.h"
#include "Organ/NOrganComponent.h"

FNProcGenTaskGraph::FNProcGenTaskGraph(UNProcGenOperation* Operation, FNProcGenOperationContext* Context)
{
	// Convert our friendly seed to something more appropriate
	const uint64 BaseSeed = FNSeedGenerator::SeedFromFriendlySeed(Context->GetOperationSettings().Seed);
	UE_LOG(LogNexusProcGen, Log, TEXT("Converted friendly seed(%s) to uint64 seed(%llu)"), *Context->GetOperationSettings().Seed, BaseSeed);
	FNMersenneTwister BaseGenerator(BaseSeed);
	
	// We need something that each task can share context to others with
	TSharedPtr<FNProcGenTaskGraphContext> SharedContextPtr = MakeShared<FNProcGenTaskGraphContext, ESPMode::ThreadSafe>(
		Context->GetTargetWorld(), Context->GetOperationSettings());
	
	int PassCount = 0;
	// Build out the organ generation tasks, with finalizers
	for (auto Pass : Context->GenerationOrder)
	{
		TSharedPtr<FNOrganGeneratorPassContext> PassContextPtr = MakeShared<FNOrganGeneratorPassContext, ESPMode::ThreadSafe>();
		
		int ComponentCount = 0;
		FGraphEventArray Tasks;
		for (const auto Component : Pass)
		{
			// Get the context generated during pre-generation process
			FNProcGenOperationOrganContext* ContextMap = Context->OrganContext.Find(Component);
			
			// Do not generate a component if it is not activated, don't make the tasks
			if (!ContextMap->SourceComponent->bActivated) continue;
			
			// Create individual organ generator context object, this builds out a list of all available cells to use to fill the space
			TSharedPtr<FNProcGenGraphBuilderContext> ContextPtr = MakeShared<FNProcGenGraphBuilderContext>(
				ContextMap, BaseGenerator.UnsignedInteger64(), FString::Printf(TEXT("%i:%i_%s"), PassCount, ComponentCount, *Component->GetDebugLabel()));
			
			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganGraphBuilderTask = TGraphTask<FNProcGenGraphBuilderTask>::CreateTask(
				(PassTasks.Num() > 0) ? &PassTasks.Last() : nullptr, 
				ENamedThreads::AnyNormalThreadNormalTask) // Doesnt need to run on main thread
				.ConstructAndHold(ContextPtr, PassContextPtr, SharedContextPtr); // ConstructAndDispatchWhenReady
			Tasks.Add(OrganGraphBuilderTask);
			
			// Create a task to finalize the previous organ task on the main thread
			FGraphEventArray SubTasks;
			SubTasks.Add(OrganGraphBuilderTask);
			FGraphEventRef OrganFinalizeTask = TGraphTask<FNOrganGeneratorFinalizeTask>::CreateTask(&SubTasks, 
				ENamedThreads::GameThread)
				.ConstructAndHold(Operation, ContextPtr, PassContextPtr, SharedContextPtr);
			Tasks.Add(OrganFinalizeTask);
			
			ComponentCount++;
		}
		PassCount++;
		
		/// Ensure we track a passes tasksman 
		PassTasks.Add(Tasks);
	};//
	
	// TODO: Validate task to ensure generation is completable?
	
	// Create our finalizer task on the main thread
	FinalizeTask = TGraphTask<FNProcGenOperationFinalizeTask>::CreateTask(&PassTasks.Last(), ENamedThreads::GameThread).ConstructAndHold(Operation, SharedContextPtr);
}

void FNProcGenTaskGraph::UnlockTasks()
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

void FNProcGenTaskGraph::WaitForTasks()
{
	if (!bTasksUnlocked)
	{
		UnlockTasks();
	}

	FTaskGraphInterface::Get().WaitUntilTaskCompletes(FinalizeTask);
}

void FNProcGenTaskGraph::ResetGraph()
{
	bTasksUnlocked = false;
}

int FNProcGenTaskGraph::GetTotalPasses() const
{
	return PassTasks.Num();
}

int FNProcGenTaskGraph::GetCompletedPasses()
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

FIntVector2 FNProcGenTaskGraph::GetTaskStatus() const
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