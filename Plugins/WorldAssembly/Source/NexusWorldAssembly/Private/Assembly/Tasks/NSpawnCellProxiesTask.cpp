// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NSpawnCellProxiesTask.h"


void FNSpawnCellProxiesTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{

	N_ASSEMBLY_ANALYTICS_INDEX_DEFINE(SpawnCellProxiesCreate)
	N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesStart)

	// Reports the spawn pass's status channel (created/spawned fraction) so the combined progress bar keeps
	// moving across timeslices. The channel id lives on the shared context so it survives re-dispatches.
	auto ReportSpawnChannel = [&](const FString& Message, const bool bClose)
	{
		int32& ChannelId = SpawnCellsContextPtr->SpawnStatusChannelId;
		if (ChannelId == INDEX_NONE) return;

		const int32 Total = SpawnCellsContextPtr->CellNodes.Num();
		const float Percent = Total > 0 ? static_cast<float>(SpawnCellsContextPtr->CellNodesCreateCurrentIndex) / static_cast<float>(Total) : 0.f;
		TaskGraphContextPtr->SetChannelStatus(ChannelId, Message, Percent);
		if (bClose)
		{
			TaskGraphContextPtr->CloseStatusChannel(ChannelId);
			ChannelId = INDEX_NONE;
		}
	};

	// Cancellation check — the operation may have been cancelled between timeslice dispatches
	if (SpawnCellsContextPtr->bCancelled.Load())
	{
		ReportSpawnChannel(TEXT("Cancelled"), true);
		CompletionEvent->Unlock();
		return;
	}

	// Dispatch Guard #1
	UWorld* TargetWorld = SpawnCellsContextPtr->World;
	if (!IsValid(TargetWorld) || TargetWorld->bIsTearingDown)
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Skipping dispatched FNSpawnCellProxiesTask as the World is in a bad state (PIE, teardown, etc.). Any associated reports SpawnCellProxies timers will be incorrect."));
		ReportSpawnChannel(TEXT("Aborted"), true);
		CompletionEvent->Unlock();
		return;
	}

	const double MaxAllowableTime = SpawnCellsContextPtr->CellTimeSlice;
	const double StartTime = FPlatformTime::Seconds();
	const int32 NodeCount = SpawnCellsContextPtr->CellNodes.Num();

	// Copy local
	const FGameplayTagContainer ContextTags = TaskGraphContextPtr->ContextTags;
	const TArray<FNGameplayTagCount> TagCounter = TaskGraphContextPtr->TagCounter.ToTagCount();

	// Early out when we do not have any nodes to spawn
	if (NodeCount == 0)
	{
		N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesFinish)
		CompletionEvent->Unlock();
		return;
	}

	// Open the spawn channel on the first dispatch; reused across timeslices until the pass completes.
	if (SpawnCellsContextPtr->SpawnStatusChannelId == INDEX_NONE)
	{
		SpawnCellsContextPtr->SpawnStatusChannelId = TaskGraphContextPtr->OpenStatusChannel(TEXT("Spawning Cells"));
	}

	FNCellAssemblyData CellAssemblyData;
	CellAssemblyData.OperationTicket = SpawnCellsContextPtr->OperationTicket;
	CellAssemblyData.ContextTags = ContextTags;
	CellAssemblyData.TagCounter = TagCounter;

	for (int32 i = SpawnCellsContextPtr->CellNodesCreateCurrentIndex; i < NodeCount; i++)
	{
		FNAssemblyGraphCellNode* CellNode = SpawnCellsContextPtr->CellNodes[i];

		// Instance specific stuff
		CellAssemblyData.Seed = CellNode->GetSeed();
		CellAssemblyData.NodeIdentifier = CellNode->GetNodeIdentifier();
		CellAssemblyData.AssemblyTags = CellNode->GetAssemblyTags();
		CellAssemblyData.ContextTagsAdded = CellNode->GetContextTagsAdded();
		CellAssemblyData.LinkDetails = CellNode->GetLinkDetails();
		CellAssemblyData.bHotPathShortest = CellNode->IsHotPathShortest();
		CellAssemblyData.bHotPathSequential = CellNode->IsHotPathSequential();

		// Convert to basic array
		CellNode->GetJunctions().GenerateValueArray(CellAssemblyData.JunctionDetails);

		// Create the instance
		ANCellProxy* Proxy = ANCellProxy::CreateInstance(SpawnCellsContextPtr->World, CellNode, CellAssemblyData, SpawnCellsContextPtr->bPreloadLevels);

		// Dispatch Guard #2
		if (Proxy == nullptr)
		{
			// SpawnActor failed (e.g. world began tearing down mid-batch); abandon remaining nodes.
			UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Bailing on FNSpawnCellProxiesTask as it was not able to spawn a proxy, most likely cause the World is in a bad state. Any associated reports SpawnCellProxies timers will be incorrect."));
			ReportSpawnChannel(TEXT("Unsuccessful"), true);
			CompletionEvent->Unlock();
			return;
		}

		// Registered with global?
		TaskGraphContextPtr->CreatedProxies.Add(Proxy);

		// What about creating the instance?
		if (SpawnCellsContextPtr->bSpawnLevelInstances)
		{
			Proxy->CreateLevelInstance();
			// TODO: This  could be a separate pass to not block on loading the level, get all the actors in place
			Proxy->LoadLevelInstance();
		}

		SpawnCellsContextPtr->CellNodesCreateCurrentIndex++;
		N_ASSEMBLY_ANALYTICS_TWO_PARAM(SpawnCellProxiesSpawned, N_ASSEMBLY_ANALYTICS_MEMBER_INDEX, SpawnCellsContextPtr->CellNodes[i]->GetTemplate()->GetFName())

		if (SpawnCellsContextPtr->CellNodesCreateCurrentIndex == NodeCount)
		{
			N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesFinish)
			ReportSpawnChannel(TEXT("Complete"), true);
			CompletionEvent->Unlock(); // Triggers
			return;
		}

		// Outside of the required slicing, dispatch a new task that should move on to the next frame?
		if (FPlatformTime::Seconds() - StartTime > MaxAllowableTime)
		{
			N_ASSEMBLY_ANALYTICS_INDEX(SpawnCellProxiesFinish)
			ReportSpawnChannel(FString::Printf(TEXT("Spawned %i/%i"), SpawnCellsContextPtr->CellNodesCreateCurrentIndex, NodeCount), false);
			TGraphTask<FNSpawnCellProxiesTask>::CreateTask(nullptr,  ENamedThreads::GameThread)
				.ConstructAndDispatchWhenReady(SpawnCellsContextPtr, TaskGraphContextPtr, CompletionEvent N_ASSEMBLY_ANALYTICS_CLASS_REF);
			return;
		}

	}
}
