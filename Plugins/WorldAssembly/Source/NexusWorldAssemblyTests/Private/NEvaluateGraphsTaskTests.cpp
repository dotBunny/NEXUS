// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Assembly/Tasks/NEvaluateGraphsTask.h"
#include "Cell/NCellAssemblyData.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNEvaluateGraphsTaskHarness
{
	/** A minimal cell carrying one junction, so link-detail generation has something to emit an entry for. */
	static FNVirtualCellData MakeCellShell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);

		FNCellJunctionDetails Junction;
		Junction.InstanceIdentifier = 1;
		Cell.Junctions.Add(0, Junction);
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBone()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/** Create and register a cell node, tagging it as a Hotpath goal when bGoal is true. */
	static FNAssemblyGraphCellNode* AddCell(FNAssemblyGraph& Graph, FNVirtualCellData& Cell, const bool bGoal)
	{
		FNAssemblyGraphNodeParams Params;
		if (bGoal)
		{
			Params.AssemblyTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("NEXUS.WorldAssembly.Flag.Hotpath")));
		}
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}
}

N_TEST_CRITICAL(FNEvaluateGraphsTaskTests_RunsEveryPass,
	"NEXUS::UnitTests::NWorldAssembly::FNEvaluateGraphsTask::RunsEveryPass",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Guards the wiring rather than the algorithms. Each of the three passes is covered in depth elsewhere against
	// FNAssemblyGraph directly, and every one of those tests would stay green if this task stopped calling one of
	// them — a dropped call surfaces as cells reaching runtime with default data, not as a red test. So this drives
	// the task itself and requires evidence that all three actually ran.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNEvaluateGraphsTaskHarness;

	FNVirtualCellData Cell = MakeCellShell();

	const FNAssemblyOperationSettings Settings;
	const TSharedPtr<FNAssemblyTaskGraphContext> Context =
		MakeShared<FNAssemblyTaskGraphContext, ESPMode::ThreadSafe>(nullptr, 1, Settings);

	// Bone - Start - Goal, plus a cell hanging one junction off the route so proximity has a non-zero score to
	// report. Built into a graph the context owns, which is where the task reads them from.
	FNAssemblyGraphBoneNode* Bone = MakeBone();
	TUniquePtr<FNAssemblyGraph> Graph = MakeUnique<FNAssemblyGraph>(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(*Graph, Cell, false);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Goal = AddCell(*Graph, Cell, true);
	Start->Connect(Goal);
	FNAssemblyGraphCellNode* OffPath = AddCell(*Graph, Cell, false);
	Start->Connect(OffPath);

	Context->TakeGraph(MoveTemp(Graph));

	N_ASSEMBLY_ANALYTICS_SHARED_PTR
#if !UE_BUILD_SHIPPING
	AnalyticsPtr = MakeShared<FNAssemblyTaskAnalytics, ESPMode::ThreadSafe>(
		FText::FromString(TEXT("FNEvaluateGraphsTaskTests")));
#endif

	FNEvaluateGraphsTask Task(Context N_ASSEMBLY_ANALYTICS_CLASS_REF);
	// Completed by hand after the call. The task graph would normally do this, and an FGraphEvent destroyed having
	// never completed trips an assert inside the engine that takes the whole run down with it.
	const FGraphEventRef CompletionEvent = FGraphEvent::CreateGraphEvent();
	Task.DoTask(ENamedThreads::AnyThread, CompletionEvent);
	CompletionEvent->DispatchSubsequents();

	// Pass 1 - hot path resolution.
	CHECK_MESSAGE(TEXT("The task must resolve the hot path: the goal cell should be flagged on both variants."),
		Goal->IsHotPathShortest() && Goal->IsHotPathSequential())
	CHECK_FALSE_MESSAGE(TEXT("A cell off the route must not be flagged."), OffPath->IsHotPath())

	// Pass 2 - proximity scoring. Asserted on the off-path cell specifically: a cell *on* the route scores 0, which
	// is also what a score would read as if it were never written at all only in the opposite direction — so the
	// meaningful evidence that scoring ran is a non-zero, non-default value one hop out.
	CHECK_EQUALS("The task must score proximity: a cell one junction off the hot path scores 1.",
		static_cast<int32>(OffPath->GetHotPathShortestScore()), 1)
	CHECK_EQUALS("The cell on the hot path scores 0.",
		static_cast<int32>(Goal->GetHotPathShortestScore()), 0)

	// Pass 3 - link details.
	CHECK_MESSAGE(TEXT("The task must generate link details: every cell should have one entry per junction."),
		Start->GetLinkDetails().Num() == 1 && Goal->GetLinkDetails().Num() == 1)
}

N_TEST_HIGH(FNEvaluateGraphsTaskTests_BailsOutWhenCancelled,
	"NEXUS::UnitTests::NWorldAssembly::FNEvaluateGraphsTask::BailsOutWhenCancelled",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cancelled operation must not pay for the evaluation. Hot path resolution is the most expensive thing in the
	// stage, so the check has to happen before it rather than only between passes.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNEvaluateGraphsTaskHarness;

	FNVirtualCellData Cell = MakeCellShell();

	const FNAssemblyOperationSettings Settings;
	const TSharedPtr<FNAssemblyTaskGraphContext> Context =
		MakeShared<FNAssemblyTaskGraphContext, ESPMode::ThreadSafe>(nullptr, 1, Settings);

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	TUniquePtr<FNAssemblyGraph> Graph = MakeUnique<FNAssemblyGraph>(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(*Graph, Cell, false);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Goal = AddCell(*Graph, Cell, true);
	Start->Connect(Goal);

	Context->TakeGraph(MoveTemp(Graph));
	Context->RequestCancel();

	N_ASSEMBLY_ANALYTICS_SHARED_PTR
#if !UE_BUILD_SHIPPING
	AnalyticsPtr = MakeShared<FNAssemblyTaskAnalytics, ESPMode::ThreadSafe>(
		FText::FromString(TEXT("FNEvaluateGraphsTaskTests")));
#endif

	FNEvaluateGraphsTask Task(Context N_ASSEMBLY_ANALYTICS_CLASS_REF);
	// Completed by hand after the call. The task graph would normally do this, and an FGraphEvent destroyed having
	// never completed trips an assert inside the engine that takes the whole run down with it.
	const FGraphEventRef CompletionEvent = FGraphEvent::CreateGraphEvent();
	Task.DoTask(ENamedThreads::AnyThread, CompletionEvent);
	CompletionEvent->DispatchSubsequents();

	CHECK_FALSE_MESSAGE(TEXT("A cancelled task must not resolve the hot path."), Goal->IsHotPath())
	CHECK_EQUALS("A cancelled task must leave proximity scores at their default.",
		static_cast<int32>(Goal->GetHotPathShortestScore()),
		static_cast<int32>(FNCellAssemblyData::UnreachableScore))
	CHECK_EQUALS("A cancelled task must not generate link details.", Start->GetLinkDetails().Num(), 0)
}

#endif //WITH_TESTS
