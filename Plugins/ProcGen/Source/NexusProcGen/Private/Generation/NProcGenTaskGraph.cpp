// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenTaskGraph.h"

#include "NProcGenMinimal.h"

#include "Generation/NProcGenOperationContext.h"
#include "Generation/Tasks/NCollectPassContext.h"
#include "Generation/Tasks/NOrganGraphBuilderTask.h"
#include "Generation/Tasks/NProcGenFinalizeTask.h"
#include "Generation/NProcGenTaskGraphContext.h"
#include "Generation/Tasks/NCollectPassTask.h"
#include "Generation/Tasks/NSpawnCellProxiesTask.h"

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
	TSharedPtr<FNProcGenTaskGraphContext> TaskGraphContextPtr = MakeShared<FNProcGenTaskGraphContext, ESPMode::ThreadSafe>(
		Context->GetTargetWorld(), Context->GetOperationSettings());
	
	// ----------------------------------------------------------------------------------------------------------------------------------------------
	// STEP 1 - BUILD CELL GRAPHS FOR ORGANS
	// ----------------------------------------------------------------------------------------------------------------------------------------------
	int PassCount = 0;
	FGraphEventRef LastPassTask = nullptr;
	
	// Build out the organ generation tasks, with finalizers
	
	for (auto Pass : Context->GenerationOrder)
	{
		// Create context for the pass itself that organ builders will hand off their generated graph too
		TSharedPtr<FNCollectPassContext> PassContextPtr = MakeShared<FNCollectPassContext, ESPMode::ThreadSafe>();
		
		int ComponentCount = 0;
		FGraphEventArray PassTasks;
		for (const auto Component : Pass)
		{
			// Get the context generated during pre-generation process
			FNProcGenOperationOrganContext* ContextMap = Context->OrganContext.Find(Component);
			
			// Do not generate a component if it is not activated, don't make the tasks
			if (!ContextMap->SourceComponent->bActivated) continue;
			
			// Create individual organ builder context object, building out a list of all available cells to use to fill the defined space
			TSharedPtr<FNOrganGraphBuilderContext> ContextPtr = MakeShared<FNOrganGraphBuilderContext>(
				ContextMap, BaseGenerator.UnsignedInteger64(), 
				FString::Printf(TEXT("%i:%i_%s"), PassCount, ComponentCount, *Component->GetDebugLabel()));
			
			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganGraphBuilderTask = TGraphTask<FNOrganGraphBuilderTask>::CreateTask(
				(GraphBuilderTasks.Num() > 0) ? &GraphBuilderTasks.Last() : nullptr,  // Ensures we are waiting for the last pass to complete
				ENamedThreads::AnyNormalThreadNormalTask) // Doesn't need to run on game thread
				.ConstructAndHold(ContextPtr, PassContextPtr);
			PassTasks.Add(OrganGraphBuilderTask);
			AllTasks.Add(OrganGraphBuilderTask);

			ComponentCount++;
		}
		//  Record builder tasks to create dependency on previous phase
		GraphBuilderTasks.Add(PassTasks);
		
		// Create task that will when all organ tasks are complete for this pass, collect their created graphs and move them upstream.
		FGraphEventRef CollectTask = TGraphTask<FNCollectPassTask>::CreateTask(&PassTasks, ENamedThreads::AnyBackgroundThreadNormalTask)
			.ConstructAndHold(PassContextPtr, TaskGraphContextPtr);
		CollectionTasks.Add(CollectTask);
		AllTasks.Add(CollectTask);
		
		// Increment pass count for labeling
		PassCount++;
	};
	
	// TODO: Validate task to ensure generation is completable?
	
	FGraphEventRef SpawnCellProxiesTask = TGraphTask<FNSpawnCellProxiesTask>::CreateTask(&CollectionTasks, ENamedThreads::GameThread).ConstructAndHold(TaskGraphContextPtr);
	FinalizerTasks.Add(SpawnCellProxiesTask);
	AllTasks.Add(SpawnCellProxiesTask);
	
	// Create our finalizer task on the main thread that will wait for all of the other finalizers to complete
	FinalizeTask = TGraphTask<FNProcGenFinalizeTask>::CreateTask(&FinalizerTasks, ENamedThreads::GameThread).ConstructAndHold(Operation, TaskGraphContextPtr);
	AllTasks.Add(FinalizeTask);
}

void FNProcGenTaskGraph::UnlockTasks()
{
	// Start running the tasks
	// We can unlock in order as they would have been created in order
	for (const auto Task  : AllTasks)
	{
		Task->Unlock();
	}
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

FIntVector2 FNProcGenTaskGraph::GetTaskStatus() const
{
	int TotalTasks = 0;
	int CompletedTasks = 0;
	for (const FGraphEventRef& Task  : AllTasks)
	{
		if (Task->IsComplete())
		{
			CompletedTasks++;
		}
		TotalTasks++;
	}
	return FIntVector2(CompletedTasks, TotalTasks);
}