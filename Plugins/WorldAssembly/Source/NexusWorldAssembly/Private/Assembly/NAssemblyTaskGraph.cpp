// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NAssemblyTaskGraph.h"

#include "NWorldAssemblyMinimal.h"
#include "Assembly/NAssemblyOperation.h"

#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Assembly/Contexts/NPassContext.h"
#include "Assembly/Tasks/NOrganGraphBuilderTask.h"
#include "Assembly/Tasks/NAssemblyFinalizeTask.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/Contexts/NSpawnContext.h"
#include "Assembly/Tasks/NProcessPassTask.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Assembly/Tasks/NProcessVirtualWorldTask.h"
#include "Assembly/Tasks/NCreateSpawnsTask.h"
#include "Assembly/Tasks/NSpawnCellProxiesTask.h"

#include "Math/NMersenneTwister.h"
#include "Math/NSeedGenerator.h"
#include "Organ/NOrganComponent.h"

FNAssemblyTaskGraph::FNAssemblyTaskGraph(UNAssemblyOperation* Operation, FNAssemblyOperationContext* Context)
{
	// Create our analytics object
	N_ASSEMBLY_ANALYTICS_CREATE
	N_ASSEMBLY_ANALYTICS(TaskGraphCreationStart)
	
	// Convert our friendly seed to something more appropriate
	const uint64 BaseSeed = FNSeedGenerator::SeedFromFriendlySeed(Context->GetOperationSettings().Seed);
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Converted friendly seed(%s) to uint64 seed(%llu)"), *Context->GetOperationSettings().Seed, BaseSeed);
	FNMersenneTwister BaseGenerator(BaseSeed);
	
	// We need something that each task can share context to others with
	TaskGraphContextPtr = MakeShared<FNAssemblyTaskGraphContext, ESPMode::ThreadSafe>(
		Context->GetTargetWorld(), Context->GetOperationTicket(), Context->GetOperationSettings());

	// ----- STEP 0 - CAPTURE WORLD (GAME THREAD) ---------------------------------------------------------------------------------------------------

	// Create our world context holder
	VirtualWorldContextPtr = MakeShared<FNVirtualWorldContext, ESPMode::ThreadSafe>(Context->GetTargetWorld(), Context->Bounds);

	// Create our base world evaluation that builds out the collision-mesh for the world.
	FGraphEventRef CreateVirtualWorldTask = TGraphTask<FNCreateVirtualWorldTask>::CreateTask(
				nullptr,
				FNCreateVirtualWorldTask::GetDesiredThread())
				.ConstructAndHold(VirtualWorldContextPtr N_ASSEMBLY_ANALYTICS_CLASS_REF);
	PreGameThreadTasks.Add(CreateVirtualWorldTask);
	AllTasks.Add(CreateVirtualWorldTask);

	// ----- STEP 1 - PROCESS WORLD CAPTURE ---------------------------------------------------------------------------------------------------------
	
	// Create associated data from our initial game-thread blocking task
	FGraphEventRef ProcessVirtualWorldContextTask = TGraphTask<FNProcessVirtualWorldTask>::CreateTask(
				&PreGameThreadTasks,
				FNProcessVirtualWorldTask::GetDesiredThread())
				.ConstructAndHold(VirtualWorldContextPtr N_ASSEMBLY_ANALYTICS_CLASS_REF);
	ProcessInitialGameThreadTasks.Add(ProcessVirtualWorldContextTask);
	AllTasks.Add(ProcessVirtualWorldContextTask);
	
	// ----- STEP 2 - BUILD CELL GRAPHS FOR ORGANS --------------------------------------------------------------------------------------------------
	
	int32 PassCount = 0;

	// Iterate over the different bespoke passes that can occur and create dependency-chained tasks
	for (auto PassComponents : Context->GenerationOrder)
	{
		// Create context for the pass itself that organ builders will hand off their generated graph too
		TSharedPtr<FNPassContext> PassContextPtr = MakeShared<FNPassContext, ESPMode::ThreadSafe>();

		int32 ComponentCount = 0;
		FGraphEventArray PassTasks;
		for (const auto Component : PassComponents)
		{
			// Get the data generated during pre-generation process
			FNWorldOrganData* WorldOrganData = Context->OrganData.Find(Component);

			// Do not generate a component if it is not activated, don't make the tasks
			if (!WorldOrganData->SourceComponent->bActivated) continue;

			// Create individual organ builder context object, building out a list of all available cells to use to fill the defined space
			TSharedPtr<FNVirtualOrganContext> VirtualOrganContextPtr = MakeShared<FNVirtualOrganContext>(
				WorldOrganData, BaseGenerator.UnsignedInteger64(),
				FString::Printf(TEXT("%i:%i_%s"), PassCount, ComponentCount, *Component->GetDebugLabel()));

			for (const FNVirtualCellData& CellData : VirtualOrganContextPtr->CellInputData)
			{
				if (CellData.Template != nullptr)
				{
					// We hold the references here as they are pinned against GC
					Operation->ReferencedCells.AddUnique(CellData.Template);
				}
			}

			// Create a task and pass the context to the constructor. Chain onto the previous pass's FNProcessPassTask
			// (not its organ-builders) so this pass sees the previous pass's collision data fully propagated into
			// the world context before any organ-builder reads NodeCollisionMeshes.
			FGraphEventRef OrganGraphBuilderTask = TGraphTask<FNOrganGraphBuilderTask>::CreateTask(
				(ProcessPassTasks.Num() > 0) ? &ProcessPassTasks.Last() : &ProcessInitialGameThreadTasks,
				FNOrganGraphBuilderTask::GetDesiredThread())
				.ConstructAndHold(VirtualOrganContextPtr, PassContextPtr, VirtualWorldContextPtr N_ASSEMBLY_ANALYTICS_CLASS_REF);
			PassTasks.Add(OrganGraphBuilderTask);
			AllTasks.Add(OrganGraphBuilderTask);

			ComponentCount++;
		}

		// We still will keep the pass count, just don't do anything else
		if (PassTasks.IsEmpty())
		{
			PassCount++;
			continue;
		}

		// Create task that will when all organ tasks are complete for this pass, collect their created graphs and move them upstream as well as
		// any additional data like collisions, etc.
		FGraphEventRef ProcessPassTask = TGraphTask<FNProcessPassTask>::CreateTask(&PassTasks, FNProcessPassTask::GetDesiredThread())
			.ConstructAndHold(PassContextPtr, VirtualWorldContextPtr, TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CLASS_REF);
		CollectionTasks.Add(ProcessPassTask);
		AllTasks.Add(ProcessPassTask);

		// Record this pass's process task so the next pass can chain onto it (it's the task that propagates collision
		// meshes/locations into the shared world context).
		FGraphEventArray& PassProcessGate = ProcessPassTasks.AddDefaulted_GetRef();
		PassProcessGate.Add(ProcessPassTask);

		// Increment pass count for labeling
		PassCount++;
	};
	
	// TODO: Validate task to ensure generation is completable?
	
	const FNAssemblyOperationSettings& OperatingSettings = Context->GetOperationSettings();
	
	// Create our context of what we are going to need to spawn back on the game-thread
	SpawnContextPtr = MakeShared<FNSpawnContext>(Context->GetTargetWorld(), Context->GetOperationTicket(),
		OperatingSettings.bPreLoadLevelInstances, OperatingSettings.bCreateLevelInstances, 
		(OperatingSettings.CellSpawnTimeSlice * 0.001f)); // Convert to expected timescale

	FGraphEventRef CreateSpawnsTask = TGraphTask<FNCreateSpawnsTask>::CreateTask(&CollectionTasks, FNCreateSpawnsTask::GetDesiredThread())
		.ConstructAndHold(SpawnContextPtr, TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CLASS_REF);
	FinalizerTasks.Add(CreateSpawnsTask);
	SpawnContextTasks.Add(CreateSpawnsTask);
	AllTasks.Add(CreateSpawnsTask);
	
	// Create our dispatcher task that will time-slice spawning
	FGraphEventRef SpawnCellProxiesTaskCompleted = FGraphEvent::CreateGraphEvent();
	FGraphEventRef SpawnCellProxiesTask = TGraphTask<FNSpawnCellProxiesTask>::CreateTask(&SpawnContextTasks, FNSpawnCellProxiesTask::GetDesiredThread())
		.ConstructAndHold(SpawnContextPtr, TaskGraphContextPtr, SpawnCellProxiesTaskCompleted N_ASSEMBLY_ANALYTICS_CLASS_REF);
	FinalizerTasks.Add(SpawnCellProxiesTask);
	FinalizerTasks.Add(SpawnCellProxiesTaskCompleted); // This is what will actually trigger moving on post spawning
	AllTasks.Add(SpawnCellProxiesTask);
	
	// Create our finalizer task on the main thread that will wait for all the other finalizers to complete and output analytics
	FinalizeTask = TGraphTask<FNAssemblyFinalizeTask>::CreateTask(&FinalizerTasks, FNAssemblyFinalizeTask::GetDesiredThread()).ConstructAndHold(Operation, TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CLASS_REF);
	AllTasks.Add(FinalizeTask);
	
	// Record end time
	N_ASSEMBLY_ANALYTICS(TaskGraphCreationFinish)
}

void FNAssemblyTaskGraph::Cancel()
{
	if (SpawnContextPtr.IsValid())
	{
		SpawnContextPtr->bCancelled.Store(true);
	}
	if (TaskGraphContextPtr.IsValid())
	{
		TaskGraphContextPtr->CreatedProxies.Empty();
	}
}

void FNAssemblyTaskGraph::UnlockTasks()
{
	// Start running the tasks
	// We can unlock in order as they would have been created in order
	for (const auto Task  : AllTasks)
	{
		Task->Unlock();
	}
	bTasksUnlocked = true;
}

void FNAssemblyTaskGraph::WaitForTasks()
{
	if (!bTasksUnlocked)
	{
		UnlockTasks();
	}
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(FinalizeTask);
}

void FNAssemblyTaskGraph::TearDownGraph()
{
	// Remove all references
	AllTasks.Empty();

	PreGameThreadTasks.Empty();
	ProcessInitialGameThreadTasks.Empty();
	ProcessPassTasks.Empty();
	CollectionTasks.Empty();
	SpawnContextTasks.Empty();

	FinalizerTasks.Empty();
	FinalizeTask = nullptr;

	// Null out world pointers before releasing shared contexts so they don't
	// dangle if ref-counted copies outlive the operation teardown.
	if (TaskGraphContextPtr.IsValid())
	{
		TaskGraphContextPtr->TargetWorld = nullptr;
	}
	if (VirtualWorldContextPtr.IsValid())
	{
		VirtualWorldContextPtr->InputWorld = nullptr;
	}
	if (SpawnContextPtr.IsValid())
	{
		SpawnContextPtr->World = nullptr;
	}
	TaskGraphContextPtr.Reset();
	VirtualWorldContextPtr.Reset();
	SpawnContextPtr.Reset();
	
	N_ASSEMBLY_ANALYTICS_DELETE
	
	// Reset flag
	bTasksUnlocked = false;
}

FIntVector2 FNAssemblyTaskGraph::GetTaskStatus() const
{
	int32 TotalTasks = 0;
	int32 CompletedTasks = 0;
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