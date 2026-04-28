// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenTaskGraph.h"

#include "NProcGenMinimal.h"
#include "NProcGenOperation.h"

#include "Generation/Contexts/NProcGenOperationContext.h"
#include "Generation/Contexts/NGenerationPassContext.h"
#include "Generation/Tasks/NOrganGraphBuilderTask.h"
#include "Generation/Tasks/NProcGenFinalizeTask.h"
#include "Generation/Contexts/NProcGenTaskGraphContext.h"
#include "Generation/Tasks/NCollectGenerationPassesTask.h"
#include "Generation/Contexts/NWorldContext.h"
#include "Generation/Tasks/NCreateWorldContextTask.h"
#include "Generation/Tasks/NProcessWorldContextTask.h"
#include "Generation/Tasks/NSpawnCellProxiesTask.h"

#include "Math/NMersenneTwister.h"
#include "Math/NSeedGenerator.h"
#include "Organ/NOrganComponent.h"

FNProcGenTaskGraph::FNProcGenTaskGraph(UNProcGenOperation* Operation, FNProcGenOperationContext* Context)
{
	// Create our analytics object - we need to make it regardless of build configuration
	Analytics = MakeShared<FNProcGenTaskAnalytics, ESPMode::ThreadSafe>();
#if !UE_BUILD_SHIPPING	
	Analytics->TaskGraphCreationStart();
#endif // !UE_BUILD_SHIPPING
	
	// Convert our friendly seed to something more appropriate
	const uint64 BaseSeed = FNSeedGenerator::SeedFromFriendlySeed(Context->GetOperationSettings().Seed);
	UE_LOG(LogNexusProcGen, Log, TEXT("Converted friendly seed(%s) to uint64 seed(%llu)"), *Context->GetOperationSettings().Seed, BaseSeed);
	FNMersenneTwister BaseGenerator(BaseSeed);
	
	// We need something that each task can share context to others with
	TSharedPtr<FNProcGenTaskGraphContext> TaskGraphContextPtr = MakeShared<FNProcGenTaskGraphContext, ESPMode::ThreadSafe>(
		Context->GetTargetWorld(), Context->GetOperationTicket(), Context->GetOperationSettings());
	
	// ----- STEP 0 - CAPTURE WORLD (GAME THREAD) ---------------------------------------------------------------------------------------------------
	
	// Create our world context holder
	TSharedPtr<FNWorldContext> WorldContextPtr = MakeShared<FNWorldContext, ESPMode::ThreadSafe>(Context->GetTargetWorld(), Context->Bounds);

	// Create our base world evaluation that builds out the collision-mesh for the world.
	FGraphEventRef CreateWorldContextTask = TGraphTask<FNCreateWorldContextTask>::CreateTask(
				nullptr,
				ENamedThreads::GameThread)
				.ConstructAndHold(WorldContextPtr, Analytics);
	Step0Tasks.Add(CreateWorldContextTask);
	AllTasks.Add(CreateWorldContextTask);

	// ----- STEP 1 - PROCESS WORLD CAPTURE ---------------------------------------------------------------------------------------------------------
	
	// Create associated data from our initial game-thread blocking task
	FGraphEventRef ProcessWorldContextTask = TGraphTask<FNProcessWorldContextTask>::CreateTask(
				&Step0Tasks,
				ENamedThreads::AnyNormalThreadNormalTask) // Doesn't need to run on game thread
				.ConstructAndHold(WorldContextPtr, Analytics);
	Step1Tasks.Add(ProcessWorldContextTask);
	AllTasks.Add(ProcessWorldContextTask);
	
	// ----- STEP 2 - BUILD CELL GRAPHS FOR ORGANS --------------------------------------------------------------------------------------------------
	
	int PassCount = 0;

	// Iterate over the different bespoke passes that can occur and create dependency-chained tasks
	for (auto Pass : Context->GenerationOrder)
	{
		// Create context for the pass itself that organ builders will hand off their generated graph too
		TSharedPtr<FNGenerationPassContext> GenerationPassContextPtr = MakeShared<FNGenerationPassContext, ESPMode::ThreadSafe>();

		int ComponentCount = 0;
		FGraphEventArray PassTasks;
		for (const auto Component : Pass)
		{
			// Get the context generated during pre-generation process
			FNOrganLockedData* ContextMap = Context->OrganContext.Find(Component);

			// Do not generate a component if it is not activated, don't make the tasks
			if (!ContextMap->SourceComponent->bActivated) continue;

			// Create individual organ builder context object, building out a list of all available cells to use to fill the defined space
			TSharedPtr<FNOrganContext> OrganContextPtr = MakeShared<FNOrganContext>(
				ContextMap, BaseGenerator.UnsignedInteger64(),
				FString::Printf(TEXT("%i:%i_%s"), PassCount, ComponentCount, *Component->GetDebugLabel()));

			// Create a task and pass the context to the constructor, as well as the previous event array if there
			FGraphEventRef OrganGraphBuilderTask = TGraphTask<FNOrganGraphBuilderTask>::CreateTask(
				(GraphBuilderTasks.Num() > 0) ? &GraphBuilderTasks.Last() : &Step1Tasks,  // Ensures we are waiting for the last pass to complete
				ENamedThreads::AnyNormalThreadNormalTask) // Doesn't need to run on game thread
				.ConstructAndHold(OrganContextPtr, GenerationPassContextPtr, WorldContextPtr, Analytics);
			PassTasks.Add(OrganGraphBuilderTask);
			AllTasks.Add(OrganGraphBuilderTask);

			ComponentCount++;
		}

		// We still will keep the pass count, just don't do anything else
		if (PassTasks.Num() == 0)
		{
			PassCount++;
			continue;
		}

		// Record builder tasks so the next pass can chain onto them.
		GraphBuilderTasks.Add(PassTasks);

		// Create task that will when all organ tasks are complete for this pass, collect their created graphs and move them upstream.
		FGraphEventRef CollectTask = TGraphTask<FNCollectGenerationPassesTask>::CreateTask(&PassTasks, ENamedThreads::AnyBackgroundThreadNormalTask)
			.ConstructAndHold(GenerationPassContextPtr, TaskGraphContextPtr, Analytics);
		CollectionTasks.Add(CollectTask);
		AllTasks.Add(CollectTask);

		// Increment pass count for labeling
		PassCount++;
	};
	
	// TODO: Validate task to ensure generation is completable?
	
	FGraphEventRef SpawnCellProxiesTask = TGraphTask<FNSpawnCellProxiesTask>::CreateTask(&CollectionTasks, ENamedThreads::GameThread).ConstructAndHold(TaskGraphContextPtr, Analytics);
	FinalizerTasks.Add(SpawnCellProxiesTask);
	AllTasks.Add(SpawnCellProxiesTask);
	
	// Create our finalizer task on the main thread that will wait for all the other finalizers to complete
	FinalizeTask = TGraphTask<FNProcGenFinalizeTask>::CreateTask(&FinalizerTasks, ENamedThreads::GameThread).ConstructAndHold(Operation, TaskGraphContextPtr, Analytics);
	AllTasks.Add(FinalizeTask);
	
	// Record end time
#if !UE_BUILD_SHIPPING	
	Analytics->TaskGraphCreationFinish();
#endif // !UE_BUILD_SHIPPING
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

void FNProcGenTaskGraph::TearDownGraph()
{
	// Remove all references
	AllTasks.Empty();
	CollectionTasks.Empty();
	FinalizerTasks.Empty();
	GraphBuilderTasks.Empty();
	FinalizeTask = nullptr;
	
	Analytics.Reset();
	
	
	// Reset flag
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