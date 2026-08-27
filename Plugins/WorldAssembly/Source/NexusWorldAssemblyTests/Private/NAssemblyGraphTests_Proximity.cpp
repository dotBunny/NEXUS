// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Cell/NCellAssemblyData.h"
#include "Cell/NCellLinkDetails.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness
{
	/** Which flag tag a test cell carries, if any. */
	enum class ECellFlag : uint8
	{
		None,
		Hotpath,
		Important
	};

	/** A minimal cell with valid bounds/hull and no junctions — enough for connectivity-only scoring tests. */
	static FNVirtualCellData MakeCellShell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBone()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/** Add a junction under map key Key carrying the persisted InstanceId, so link details have one to describe. */
	static void AddJunction(FNVirtualCellData& Cell, const int32 Key, const int32 InstanceId)
	{
		FNCellJunctionDetails Junction;
		Junction.InstanceIdentifier = InstanceId;
		Cell.Junctions.Add(Key, Junction);
	}

	/** Create and register a cell node, tagged per Flag. */
	static FNAssemblyGraphCellNode* AddCell(FNAssemblyGraph& Graph, FNVirtualCellData& Cell, const ECellFlag Flag = ECellFlag::None)
	{
		FNAssemblyGraphNodeParams Params;
		// Requested by name rather than the native tag symbols, which the runtime module does not export.
		switch (Flag)
		{
		case ECellFlag::Hotpath:
			Params.AssemblyTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("NEXUS.WorldAssembly.Flag.Hotpath")));
			break;
		case ECellFlag::Important:
			Params.AssemblyTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("NEXUS.WorldAssembly.Flag.Important")));
			break;
		default:
			break;
		}
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}

	/**
	 * Heap-allocate a graph rooted at Root into Graphs and return it, matching how an operation holds its graphs —
	 * which is also the shape FNAssemblyGraph::ScoreCellProximity consumes.
	 */
	static FNAssemblyGraph& AddGraph(TArray<TUniquePtr<FNAssemblyGraph>>& Graphs, FNAssemblyGraphNode* Root)
	{
		return *Graphs.Emplace_GetRef(MakeUnique<FNAssemblyGraph>(Root, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true));
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

	/** Flag every graph's hot path, then score the whole set — the order NCreateSpawnsTask runs them in. */
	static void FlagAndScore(TArray<TUniquePtr<FNAssemblyGraph>>& Graphs)
	{
		for (const TUniquePtr<FNAssemblyGraph>& Graph : Graphs)
		{
			Graph->FlagHotPath();
		}
		FNAssemblyGraph::ScoreCellProximity(Graphs);
	}

	/** Widened for comparison, so a failure message reports a number rather than a character. */
	static int32 HotPathScore(const FNAssemblyGraphCellNode* Cell) { return static_cast<int32>(Cell->GetHotPathShortestScore()); }
	static int32 SequentialScore(const FNAssemblyGraphCellNode* Cell) { return static_cast<int32>(Cell->GetHotPathSequentialScore()); }
	static int32 ImportanceScore(const FNAssemblyGraphCellNode* Cell) { return static_cast<int32>(Cell->GetImportanceScore()); }

	static constexpr int32 Unreachable = static_cast<int32>(FNCellAssemblyData::UnreachableScore);
}

N_TEST_CRITICAL(FNAssemblyGraphTests_Proximity_HotPathScoreRadiatesOutward,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::HotPathScoreRadiatesOutward",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell on the hot path scores 0; each cell further out along a dead-end branch scores one more.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Goal = AddCell(Graph, Cell, ECellFlag::Hotpath);
	Start->Connect(Goal);

	// A dead-end branch hanging off the start cell, three cells deep.
	FNAssemblyGraphCellNode* Off1 = AddCell(Graph, Cell);
	Start->Connect(Off1);
	FNAssemblyGraphCellNode* Off2 = AddCell(Graph, Cell);
	Off1->Connect(Off2);
	FNAssemblyGraphCellNode* Off3 = AddCell(Graph, Cell);
	Off2->Connect(Off3);

	FlagAndScore(Graphs);

	CHECK_EQUALS("A cell on the hot path scores 0.", HotPathScore(Start), 0)
	CHECK_EQUALS("The goal cell is on the hot path and scores 0.", HotPathScore(Goal), 0)
	CHECK_EQUALS("A cell one junction off the hot path scores 1.", HotPathScore(Off1), 1)
	CHECK_EQUALS("A cell two junctions off the hot path scores 2.", HotPathScore(Off2), 2)
	CHECK_EQUALS("A cell three junctions off the hot path scores 3.", HotPathScore(Off3), 3)
}

N_TEST_CRITICAL(FNAssemblyGraphTests_Proximity_ImportanceScoreRadiatesOutward,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::ImportanceScoreRadiatesOutward",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An Important-flagged cell scores 0, cells directly connected to it 1, cells connected to those 2 — and the
	// hot path scores stay untouched, because nothing here carries the Hotpath tag.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Important = AddCell(Graph, Cell, ECellFlag::Important);
	Start->Connect(Important);
	FNAssemblyGraphCellNode* Beyond = AddCell(Graph, Cell);
	Important->Connect(Beyond);
	FNAssemblyGraphCellNode* FurtherBeyond = AddCell(Graph, Cell);
	Beyond->Connect(FurtherBeyond);

	FlagAndScore(Graphs);

	CHECK_EQUALS("A cell flagged Important scores 0.", ImportanceScore(Important), 0)
	CHECK_EQUALS("A cell directly connected to an Important cell scores 1.", ImportanceScore(Start), 1)
	CHECK_EQUALS("The other cell directly connected to it also scores 1.", ImportanceScore(Beyond), 1)
	CHECK_EQUALS("A cell two connections from an Important cell scores 2.", ImportanceScore(FurtherBeyond), 2)
	CHECK_EQUALS("Importance scoring must not imply a hot path score.", HotPathScore(Important), Unreachable)
}

N_TEST_HIGH(FNAssemblyGraphTests_Proximity_NoSeedsLeavesEveryCellUnreachable,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::NoSeedsLeavesEveryCellUnreachable",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With neither tag anywhere in the assembly there is nothing to be near, so every score stays at its default.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell);
	Bone->Connect(Start);
	FNAssemblyGraphCellNode* Next = AddCell(Graph, Cell);
	Start->Connect(Next);

	FlagAndScore(Graphs);

	CHECK_EQUALS("No hot path means no shortest score.", HotPathScore(Start), Unreachable)
	CHECK_EQUALS("No hot path means no sequential score.", SequentialScore(Start), Unreachable)
	CHECK_EQUALS("No Important cell means no importance score.", ImportanceScore(Start), Unreachable)
	CHECK_EQUALS("The same holds for every other cell.", ImportanceScore(Next), Unreachable)
}

N_TEST_HIGH(FNAssemblyGraphTests_Proximity_NearestOfSeveralSourcesWins,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::NearestOfSeveralSourcesWins",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two Important cells at opposite ends of a chain. Every cell between them takes its distance from whichever
	// one is nearer, not from whichever was visited first.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	// ImportantA - A - Middle - B - ImportantB
	FNAssemblyGraphCellNode* ImportantA = AddCell(Graph, Cell, ECellFlag::Important);
	Bone->Connect(ImportantA);
	FNAssemblyGraphCellNode* A = AddCell(Graph, Cell);
	ImportantA->Connect(A);
	FNAssemblyGraphCellNode* Middle = AddCell(Graph, Cell);
	A->Connect(Middle);
	FNAssemblyGraphCellNode* B = AddCell(Graph, Cell);
	Middle->Connect(B);
	FNAssemblyGraphCellNode* ImportantB = AddCell(Graph, Cell, ECellFlag::Important);
	B->Connect(ImportantB);

	FlagAndScore(Graphs);

	CHECK_EQUALS("The cell beside the first Important cell scores 1.", ImportanceScore(A), 1)
	CHECK_EQUALS("The cell beside the second Important cell also scores 1.", ImportanceScore(B), 1)
	CHECK_EQUALS("The cell between the two takes the nearer of the two distances.", ImportanceScore(Middle), 2)
}

N_TEST_HIGH(FNAssemblyGraphTests_Proximity_NonCellNodesCostNoHop,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::NonCellNodesCostNoHop",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The score counts cells, not nodes. A bone sitting between two cells is stepped through for free, so the far
	// cell is one hop away rather than two.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Important = AddCell(Graph, Cell, ECellFlag::Important);
	Bone->Connect(Important);

	// Important - MiddleBone - Across
	FNAssemblyGraphBoneNode* MiddleBone = MakeBone();
	Graph.RegisterNode(MiddleBone);
	Important->Connect(MiddleBone);
	FNAssemblyGraphCellNode* Across = AddCell(Graph, Cell);
	MiddleBone->Connect(Across);

	FlagAndScore(Graphs);

	CHECK_EQUALS("A cell reached through a bone is one cell away, not two.", ImportanceScore(Across), 1)
}

N_TEST_CRITICAL(FNAssemblyGraphTests_Proximity_VariantsScoreIndependently,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::VariantsScoreIndependently",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The topology from HotPath::ShortestAndSequentialDivergeOnShortcut: both intermediates are on the shortest
	// spokes, but the sequential chain crosses the shortcut and bypasses one of them. The bypassed intermediate
	// must therefore score 0 on one variant and 1 on the other — which is the whole reason the two are scored
	// separately rather than against the union of the flags.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Start = AddCell(Graph, Cell);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* Mid1 = AddCell(Graph, Cell);
	Start->Connect(Mid1);
	FNAssemblyGraphCellNode* Goal1 = AddCell(Graph, Cell, ECellFlag::Hotpath);
	Mid1->Connect(Goal1);

	FNAssemblyGraphCellNode* Mid2 = AddCell(Graph, Cell);
	Start->Connect(Mid2);
	FNAssemblyGraphCellNode* Goal2 = AddCell(Graph, Cell, ECellFlag::Hotpath);
	Mid2->Connect(Goal2);

	// Loop-closure shortcut between the two goals.
	Goal1->Connect(Goal2);

	FlagAndScore(Graphs);

	CHECK_EQUALS("Both intermediates sit on the shortest spokes, so both score 0 there.", HotPathScore(Mid1), 0)
	CHECK_EQUALS("The second intermediate scores 0 on the shortest variant too.", HotPathScore(Mid2), 0)

	// Which of the two the sequential chain bypasses depends on goal ordering, so assert the shape rather than
	// pinning the identity: one intermediate is on the chain, the other sits one cell off it.
	const int32 Lower = FMath::Min(SequentialScore(Mid1), SequentialScore(Mid2));
	const int32 Higher = FMath::Max(SequentialScore(Mid1), SequentialScore(Mid2));
	CHECK_EQUALS("One intermediate lies on the sequential chain.", Lower, 0)
	CHECK_EQUALS("The intermediate the shortcut bypasses sits one cell off the sequential chain.", Higher, 1)
}

N_TEST_CRITICAL(FNAssemblyGraphTests_Proximity_ScoresCrossGraphBoundaries,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::ScoresCrossGraphBoundaries",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The junction-connector pass wires node-level edges between cells in different graphs. A cell one connector
	// away from another organ's Important cell is genuinely one cell away, and must score as such.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* BoneA = MakeBone();
	FNAssemblyGraph& GraphA = AddGraph(Graphs, BoneA);
	FNAssemblyGraphCellNode* Important = AddCell(GraphA, Cell, ECellFlag::Important);
	BoneA->Connect(Important);

	FNAssemblyGraphBoneNode* BoneB = MakeBone();
	FNAssemblyGraph& GraphB = AddGraph(Graphs, BoneB);
	FNAssemblyGraphCellNode* Neighbour = AddCell(GraphB, Cell);
	BoneB->Connect(Neighbour);
	FNAssemblyGraphCellNode* Beyond = AddCell(GraphB, Cell);
	Neighbour->Connect(Beyond);

	// The cross-graph edge a connector pairing leaves behind.
	Important->Connect(Neighbour);

	FlagAndScore(Graphs);

	CHECK_EQUALS("A cell one connector from another graph's Important cell scores 1.", ImportanceScore(Neighbour), 1)
	CHECK_EQUALS("Scoring keeps counting past the graph boundary.", ImportanceScore(Beyond), 2)
}

N_TEST_MEDIUM(FNAssemblyGraphTests_Proximity_SaturatesRatherThanWrapping,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::SaturatesRatherThanWrapping",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Scores are stored as bytes. The last distance a byte can express is one below UnreachableScore; past that a
	// cell is left indistinguishable from one no seed reaches, rather than wrapping back around to 0.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	// Chain[i] sits exactly i cells from the Important cell at Chain[0].
	TArray<FNAssemblyGraphCellNode*> Chain;
	Chain.Add(AddCell(Graph, Cell, ECellFlag::Important));
	Bone->Connect(Chain[0]);
	for (int32 i = 1; i <= Unreachable + 4; i++)
	{
		Chain.Add(AddCell(Graph, Cell));
		Chain[i - 1]->Connect(Chain[i]);
	}

	FlagAndScore(Graphs);

	CHECK_EQUALS("A cell 100 hops out scores 100.", ImportanceScore(Chain[100]), 100)
	CHECK_EQUALS("The furthest expressible distance is one below UnreachableScore.",
		ImportanceScore(Chain[Unreachable - 1]), Unreachable - 1)
	CHECK_EQUALS("A cell past that reads as unreachable.", ImportanceScore(Chain[Unreachable]), Unreachable)
	CHECK_EQUALS("And so does everything beyond it.", ImportanceScore(Chain[Unreachable + 4]), Unreachable)
}

N_TEST_CRITICAL(FNAssemblyGraphTests_Proximity_LinkCarriesFarCellScores,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::LinkCarriesFarCellScores",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Each link records the scores of the cell on its far side, which is the half a junction cannot work out for
	// itself: its own cell's scores say nothing about which of that cell's doorways leads inward, and the far cell
	// is frequently not streamed in when the junction begins play.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	// Middle sits between the goal and a dead end, so its two junctions must report different far scores.
	FNVirtualCellData GoalCell = MakeCellShell();
	AddJunction(GoalCell, 0, 100);

	FNVirtualCellData MiddleCell = MakeCellShell();
	AddJunction(MiddleCell, 0, 200); // toward the goal
	AddJunction(MiddleCell, 1, 201); // toward the dead end

	FNVirtualCellData OuterCell = MakeCellShell();
	AddJunction(OuterCell, 0, 300);

	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;
	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Goal = AddCell(Graph, GoalCell, ECellFlag::Hotpath);
	Bone->Connect(Goal);
	FNAssemblyGraphCellNode* Middle = AddCell(Graph, MiddleCell);
	FNAssemblyGraphCellNode* Outer = AddCell(Graph, OuterCell);

	Goal->LinkJunction(0, Middle);
	Middle->LinkJunction(0, Goal);
	Goal->Connect(Middle);

	Middle->LinkJunction(1, Outer);
	Outer->LinkJunction(0, Middle);
	Middle->Connect(Outer);

	FlagAndScore(Graphs);
	Middle->GenerateLinkDetails();

	const FNCellLinkDetails* TowardGoal = FindByJunctionInstanceId(Middle->GetLinkDetails(), 200);
	const FNCellLinkDetails* TowardOuter = FindByJunctionInstanceId(Middle->GetLinkDetails(), 201);
	if (TowardGoal == nullptr || TowardOuter == nullptr)
	{
		ADD_ERROR("Expected link entries for both of the middle cell's junctions.");
		return;
	}

	// Middle itself scores 1; the two junctions straddle it at 0 and 2.
	CHECK_EQUALS("The junction facing the goal reports the goal's score of 0.",
		static_cast<int32>(TowardGoal->ConnectedHotPathShortestScore), 0)
	CHECK_EQUALS("The junction facing the dead end reports that cell's score of 2.",
		static_cast<int32>(TowardOuter->ConnectedHotPathShortestScore), 2)

	// Direction is the whole point: strictly lower means the doorway heads toward the route.
	CHECK_MESSAGE(TEXT("The junction facing the goal must lead toward the hot path."),
		TowardGoal->ConnectedHotPathShortestScore < Middle->GetHotPathShortestScore())
	CHECK_FALSE_MESSAGE(TEXT("The junction facing the dead end must not lead toward the hot path."),
		TowardOuter->ConnectedHotPathShortestScore < Middle->GetHotPathShortestScore())

	// The derived accessor, and its distinction from bHotPathShortest: the far cell is on the route while this
	// junction is not, because the near cell is off it. That is exactly a doorway *onto* the route.
	CHECK_MESSAGE(TEXT("The far cell of the goal-facing junction must report as on the shortest hot path."),
		TowardGoal->IsConnectedOnHotPathShortest())
	CHECK_FALSE_MESSAGE(TEXT("That junction is not itself on the hot path, since the near cell is off it."),
		TowardGoal->bHotPathShortest)
}

N_TEST_HIGH(FNAssemblyGraphTests_Proximity_UnconnectedLinkKeepsUnreachableFarScores,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::Proximity::UnconnectedLinkKeepsUnreachableFarScores",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A junction that reaches nothing has no far cell to describe, so its scores stay at the default rather than
	// reading as 0 — which would otherwise claim an opening onto empty space sits on the route.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNProximityHarness;

	FNVirtualCellData Cell = MakeCellShell();
	AddJunction(Cell, 0, 100);

	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;
	FNAssemblyGraphBoneNode* Bone = MakeBone();
	FNAssemblyGraph& Graph = AddGraph(Graphs, Bone);

	FNAssemblyGraphCellNode* Lone = AddCell(Graph, Cell, ECellFlag::Hotpath);
	Bone->Connect(Lone);

	FlagAndScore(Graphs);
	Lone->GenerateLinkDetails();

	const FNCellLinkDetails* Open = FindByJunctionInstanceId(Lone->GetLinkDetails(), 100);
	if (Open == nullptr)
	{
		ADD_ERROR("Expected a link entry for the cell's only junction.");
		return;
	}

	CHECK_EQUALS("An unconnected junction keeps the default hot path far score.",
		static_cast<int32>(Open->ConnectedHotPathShortestScore), Unreachable)
	CHECK_EQUALS("An unconnected junction keeps the default importance far score.",
		static_cast<int32>(Open->ConnectedImportanceScore), Unreachable)
	CHECK_FALSE_MESSAGE(TEXT("An unconnected junction must not report its far cell as on the hot path."),
		Open->IsConnectedOnHotPathShortest())
}

#endif //WITH_TESTS
