// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NEvaluateGraphsTask.h"

#include "NWorldAssemblyMinimal.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"

FNEvaluateGraphsTask::FNEvaluateGraphsTask(
	const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER
{
}

void FNEvaluateGraphsTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	// Early out if we've already canceled it.
	if (TaskGraphContextPtr->IsCancelled())
	{
		return;
	}

	N_ASSEMBLY_ANALYTICS(EvaluateGraphsStart)

	const TArray<TUniquePtr<FNAssemblyGraph>>& Graphs = TaskGraphContextPtr->Graphs;
	const int32 StatusChannelId = TaskGraphContextPtr->OpenStatusChannel(TEXT("Evaluating Graphs"));

	// Closes the channel on every exit path, so a cancellation between passes does not leave it open in the UI.
	auto CloseChannel = [&](const FString& Message)
	{
		TaskGraphContextPtr->SetChannelStatus(StatusChannelId, Message, 1.f);
		TaskGraphContextPtr->CloseStatusChannel(StatusChannelId);
	};

	// ----- PASS 1 - HOT PATH --------------------------------------------------------------------------------------
	// Every graph is flagged before anything reads a flag. The passes below both do, and the connector pass has
	// already linked cells across graph boundaries, so a partially flagged set would be read as a settled one.
	TaskGraphContextPtr->SetChannelStatus(StatusChannelId, TEXT("Resolving Hot Paths"), 0.f);

	N_ASSEMBLY_ANALYTICS(EvaluateGraphs_HotPathStart)
	for (const TUniquePtr<FNAssemblyGraph>& Graph : Graphs)
	{
		Graph->FlagHotPath();
	}
	N_ASSEMBLY_ANALYTICS(EvaluateGraphs_HotPathFinish)

	if (TaskGraphContextPtr->IsCancelled())
	{
		CloseChannel(TEXT("Cancelled"));
		N_ASSEMBLY_ANALYTICS(EvaluateGraphsFinish)
		return;
	}

	// ----- PASS 2 - PROXIMITY SCORING -----------------------------------------------------------------------------
	// One sweep for the whole operation rather than one per graph: connector links make cells in different graphs
	// genuine neighbours, and a per-graph sweep would report those as unreachable.
	TaskGraphContextPtr->SetChannelStatus(StatusChannelId, TEXT("Scoring Proximity"), 0.5f);

	N_ASSEMBLY_ANALYTICS(EvaluateGraphs_ProximityStart)
	FNAssemblyGraph::ScoreCellProximity(Graphs);
	N_ASSEMBLY_ANALYTICS(EvaluateGraphs_ProximityFinish)

	if (TaskGraphContextPtr->IsCancelled())
	{
		CloseChannel(TEXT("Cancelled"));
		N_ASSEMBLY_ANALYTICS(EvaluateGraphsFinish)
		return;
	}

	// ----- PASS 3 - LINK DETAILS ----------------------------------------------------------------------------------
	// Reads the flags set above — including on cells belonging to other graphs — to decide whether each junction
	// forms a connection that is itself on the hot path.
	TaskGraphContextPtr->SetChannelStatus(StatusChannelId, TEXT("Generating Link Details"), 0.75f);

	int32 CellCount = 0;
	int32 HotPathGoalCount = 0;
	int32 ImportantCellCount = 0;

	N_ASSEMBLY_ANALYTICS(EvaluateGraphs_LinkDetailsStart)
	for (const TUniquePtr<FNAssemblyGraph>& Graph : Graphs)
	{
		for (FNAssemblyGraphNode* Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);
			CellNode->GenerateLinkDetails();

			// Counted on the walk that was happening anyway. These are the tag reads, not the resolved flags: what
			// makes the hot path timer above readable is how many goals it had to thread a route through.
			CellCount++;
			if (CellNode->IsHotPathFlagged())
			{
				HotPathGoalCount++;
			}
			if (CellNode->IsImportantFlagged())
			{
				ImportantCellCount++;
			}
		}
	}
	N_ASSEMBLY_ANALYTICS(EvaluateGraphs_LinkDetailsFinish)

	N_ASSEMBLY_ANALYTICS_TWO_PARAM(EvaluateGraphs_SetCounts, Graphs.Num(), CellCount)
	N_ASSEMBLY_ANALYTICS_TWO_PARAM(EvaluateGraphs_SetSeedCounts, HotPathGoalCount, ImportantCellCount)

	CloseChannel(TEXT("Complete"));

	N_ASSEMBLY_ANALYTICS(EvaluateGraphsFinish)
}
