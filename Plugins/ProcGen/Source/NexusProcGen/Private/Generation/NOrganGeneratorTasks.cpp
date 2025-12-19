// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTasks.h"

#include "NProcGenMinimal.h"
#include "Organ/NOrganGenerationContext.h"
#include "Generation/NOrganGeneratorFinalizeUnsafeTask.h"
#include "Generation/NOrganGeneratorTask.h"
#include "Math/NMersenneTwister.h"
#include "Math/NSeedGenerator.h"

FNOrganGeneratorTasks::FNOrganGeneratorTasks(UNProcGenOperation* Generator, FNOrganGenerationContext* Context)
{
	bTasksUnlocked = false;
	
	
	// TODO: Generate NOrganGraphContext?
	
	// Convert our friendly seed to something more appropriate
	const uint64 BaseSeed = FNSeedGenerator::SeedFromString(Context->FriendlySeed);
	UE_LOG(LogNexusProcGen, Log, TEXT("Converted friendly seed(%s) to uint64 seed(%llu)"), *Context->FriendlySeed, BaseSeed);
	
	FNMersenneTwister BaseGenerator(BaseSeed);
	
	// Build out processing tasks
	for (auto Pass : Context->GenerationOrder)
	{
		FGraphEventArray Tasks;
		for (const auto Component : Pass)
		{
			// Create component context
			FNOrganGenerationContextMap* ContextMap = Context->Components.Find(Component);
			
			// Create individual organ generator context object
			FNOrganGeneratorTaskContext* ContextPtr = new FNOrganGeneratorTaskContext(ContextMap, BaseGenerator.UnsignedInteger64());
			
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
	
	// Create our finalizer task on the main thread
	FinalizeTask = TGraphTask<FNOrganGeneratorFinalizeUnsafeTask>::CreateTask(&PassTasks.Last(), ENamedThreads::GameThread).ConstructAndHold(Generator);
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

int FNOrganGeneratorTasks::GetTotalPasses() const
{
	return PassTasks.Num();
}

int FNOrganGeneratorTasks::GetCompletedPasses()
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

FIntVector2 FNOrganGeneratorTasks::GetTaskStatus() const
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