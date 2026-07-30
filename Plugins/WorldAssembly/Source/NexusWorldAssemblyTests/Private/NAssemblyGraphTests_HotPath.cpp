// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Cell/NCellLinkDetails.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness
{
	/** A minimal cell with valid bounds/hull and no junctions — enough for connectivity-only pathing tests. */
	static FNVirtualCellData MakeCellShell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		return Cell;
	}

	/** Add a junction under map key Key carrying the persisted InstanceId. */
	static void AddJunction(FNVirtualCellData& Cell, const int32 Key, const int32 InstanceId)
	{
		FNCellJunctionDetails Junction;
		Junction.InstanceIdentifier = InstanceId;
		Cell.Junctions.Add(Key, Junction);
	}

	static FNAssemblyGraphBoneNode* MakeBone()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/** Create and register a cell node; tags it as a Hotpath goal when bGoal is true. */
	static FNAssemblyGraphCellNode* AddCell(FNAssemblyGraph& Graph, FNVirtualCellData& Cell, const bool bGoal)
	{
		FNAssemblyGraphNodeParams Params;
		if (bGoal)
		{
			// Request by name rather than the native tag symbol, which the runtime module does not export.
			Params.AssemblyTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("NEXUS.WorldAssembly.Flag.Hotpath")));
		}
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}

	/** Locate the generated entry for the junction with the given InstanceIdentifier, or nullptr. */
	static const FNCellLinkDetails* FindByJunctionInstanceId(const TArray<FNCellLinkDetails>& Details, const int32 InstanceId)
	{
		for (const FNCellLinkDetails& Detail : Details)
		{
			if (Detail.JunctionInstanceIdentifier == InstanceId) return &Detail;
		}
		return nullptr;
	}
}

N_TEST_CRITICAL(FNAssemblyGraphTests_HotPath_LinearChainFlagsThrough,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::HotPath::LinearChainFlagsThrough",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A single goal at the end of a chain flags the whole route (both variants); an off-route branch stays unflagged.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNVirtualCellData Cell = MakeCellShell();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell, false);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Middle = AddCell(Graph, Cell, false);
	Start->Connect(Middle);
	FNAssemblyGraphCellNode* Goal = AddCell(Graph, Cell, true);
	Middle->Connect(Goal);
	FNAssemblyGraphCellNode* OffBranch = AddCell(Graph, Cell, false);
	Start->Connect(OffBranch);

	Graph.FlagHotPath();

	CHECK_MESSAGE(TEXT("The start cell must be on both hot path variants."),
		Start->IsHotPathShortest() && Start->IsHotPathSequential())
	CHECK_MESSAGE(TEXT("A cell between the start and the goal must be on both hot path variants."),
		Middle->IsHotPathShortest() && Middle->IsHotPathSequential())
	CHECK_MESSAGE(TEXT("The goal cell must be on both hot path variants."),
		Goal->IsHotPathShortest() && Goal->IsHotPathSequential())
	CHECK_FALSE_MESSAGE(TEXT("A branch that leads to no goal must not be flagged."),
		OffBranch->IsHotPath())
}

N_TEST_HIGH(FNAssemblyGraphTests_HotPath_NoGoalsFlagsNothing,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::HotPath::NoGoalsFlagsNothing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no Hotpath-flagged cells there is no hot path; every flag stays at its default false.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNVirtualCellData Cell = MakeCellShell();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell, false);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Next = AddCell(Graph, Cell, false);
	Start->Connect(Next);

	Graph.FlagHotPath();

	CHECK_FALSE_MESSAGE(TEXT("The start cell must not be flagged when there are no goals."), Start->IsHotPath())
	CHECK_FALSE_MESSAGE(TEXT("No cell may be flagged when there are no goals."), Next->IsHotPath())
}

N_TEST_HIGH(FNAssemblyGraphTests_HotPath_StartCellGoalIsFlagged,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::HotPath::StartCellGoalIsFlagged",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the start cell is itself a goal it lies on the hot path on both variants, even with no other goals.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNVirtualCellData Cell = MakeCellShell();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell, true);
	Bone->Connect(Start);

	Graph.FlagHotPath();

	CHECK_MESSAGE(TEXT("A start cell tagged as a goal must be on both hot path variants."),
		Start->IsHotPathShortest() && Start->IsHotPathSequential())
}

N_TEST_CRITICAL(FNAssemblyGraphTests_HotPath_ShortestAndSequentialDivergeOnShortcut,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::HotPath::ShortestAndSequentialDivergeOnShortcut",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two goals sit on separate branches with a loop-closure edge between them:
	//   Start-Mid1-Goal1, Start-Mid2-Goal2, and Goal1-Goal2.
	// Shortest spokes reach each goal directly from the start, flagging both intermediates. The sequential
	// chain reaches the first goal then hops to the second across the shortcut, bypassing one intermediate.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNVirtualCellData Cell = MakeCellShell();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell, false);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* Mid1 = AddCell(Graph, Cell, false);
	Start->Connect(Mid1);
	FNAssemblyGraphCellNode* Goal1 = AddCell(Graph, Cell, true);
	Mid1->Connect(Goal1);

	FNAssemblyGraphCellNode* Mid2 = AddCell(Graph, Cell, false);
	Start->Connect(Mid2);
	FNAssemblyGraphCellNode* Goal2 = AddCell(Graph, Cell, true);
	Mid2->Connect(Goal2);

	// Loop-closure shortcut between the two goals.
	Goal1->Connect(Goal2);

	Graph.FlagHotPath();

	CHECK_MESSAGE(TEXT("Shortest spokes must flag both intermediate cells."),
		Mid1->IsHotPathShortest() && Mid2->IsHotPathShortest())
	CHECK_MESSAGE(TEXT("The sequential chain must flag exactly one intermediate (the shortcut bypasses the other)."),
		Mid1->IsHotPathSequential() != Mid2->IsHotPathSequential())
	CHECK_MESSAGE(TEXT("The start cell must be on both variants."),
		Start->IsHotPathShortest() && Start->IsHotPathSequential())
	CHECK_MESSAGE(TEXT("Both goals must be on both variants."),
		Goal1->IsHotPathShortest() && Goal1->IsHotPathSequential()
		&& Goal2->IsHotPathShortest() && Goal2->IsHotPathSequential())
}

N_TEST_HIGH(FNAssemblyGraphTests_HotPath_BranchesToAdditionalBone,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::HotPath::BranchesToAdditionalBone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An additional bone (not the root) pulls the cells between it and the nearest already-flagged node onto the
	// hot path. The link cell would otherwise be off-path because it leads to no goal.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNVirtualCellData Cell = MakeCellShell();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell, false);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Goal = AddCell(Graph, Cell, true);
	Start->Connect(Goal);

	// A side cell that leads only to an additional bone (no goal beyond it).
	FNAssemblyGraphCellNode* Link = AddCell(Graph, Cell, false);
	Start->Connect(Link);
	FNAssemblyGraphBoneNode* ExtraBone = MakeBone();
	Graph.RegisterNode(ExtraBone);
	Link->Connect(ExtraBone);

	Graph.FlagHotPath();

	CHECK_MESSAGE(TEXT("The goal must be flagged on both variants."),
		Goal->IsHotPathShortest() && Goal->IsHotPathSequential())
	CHECK_MESSAGE(TEXT("A cell linking the hot path to an additional bone must be flagged on both variants."),
		Link->IsHotPathShortest() && Link->IsHotPathSequential())
}

N_TEST_HIGH(FNAssemblyGraphTests_HotPath_JunctionFlaggedBetweenOnPathCells,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::HotPath::JunctionFlaggedBetweenOnPathCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// GenerateLinkDetails flags a junction only when both it and the cell it links to lie on the hot path.
	// The junction to the goal is flagged; the junction to an off-path cell is not.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNHotPathHarness;

	FNVirtualCellData CellStart = MakeCellShell();
	AddJunction(CellStart, 0, 100); // links to the goal
	AddJunction(CellStart, 1, 101); // links to the off-path cell

	FNVirtualCellData CellGoal = MakeCellShell();
	AddJunction(CellGoal, 0, 200);

	FNVirtualCellData CellOff = MakeCellShell();
	AddJunction(CellOff, 0, 300);

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, CellStart, false);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Goal = AddCell(Graph, CellGoal, true);
	FNAssemblyGraphCellNode* Off = AddCell(Graph, CellOff, false);

	Start->LinkJunction(0, Goal);
	Goal->LinkJunction(0, Start);
	Start->Connect(Goal);

	Start->LinkJunction(1, Off);
	Off->LinkJunction(0, Start);
	Start->Connect(Off);

	Graph.FlagHotPath();
	Start->GenerateLinkDetails();

	const FNCellLinkDetails* ToGoal = FindByJunctionInstanceId(Start->GetLinkDetails(), 100);
	const FNCellLinkDetails* ToOff = FindByJunctionInstanceId(Start->GetLinkDetails(), 101);
	if (ToGoal == nullptr || ToOff == nullptr)
	{
		ADD_ERROR("Expected link entries for both of the start cell's junctions.");
		return;
	}

	CHECK_MESSAGE(TEXT("A junction between two on-path cells must be flagged on both variants."),
		ToGoal->bHotPathShortest && ToGoal->bHotPathSequential)
	CHECK_FALSE_MESSAGE(TEXT("A junction to an off-path cell must not be flagged on any variant."),
		ToOff->bHotPathShortest || ToOff->bHotPathSequential)
}

#endif //WITH_TESTS
