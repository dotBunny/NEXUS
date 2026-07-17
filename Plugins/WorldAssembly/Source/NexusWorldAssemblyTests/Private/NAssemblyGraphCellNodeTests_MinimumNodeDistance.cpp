// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDistanceHarness
{
	// SearchForMatchingCellInputData is what enforces MinimumNodeDistance in FilterCellInputData: production calls
	// SourceCellNode->SearchForMatchingCellInputData(Candidate, Candidate->MinimumNodeDistance - 1) and rejects the
	// candidate when it returns true. These tests drive that real method directly. Cells get two junctions so an
	// interior node can link to both a previous and a next neighbour, forming a navigable chain.
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		Cell.Junctions.Add(0, FNCellJunctionDetails());
		Cell.Junctions.Add(1, FNCellJunctionDetails());
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/** Create a cell node from Cell and register it with Graph (which then owns and frees it). */
	static FNAssemblyGraphCellNode* AddNode(FNAssemblyGraph& Graph, FNVirtualCellData& Cell)
	{
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}

	/** Bidirectionally link two nodes so the junction-walk in SearchForMatchingCellInputData can traverse both ways. */
	static void Link(FNAssemblyGraphCellNode* A, const int32 KeyA, FNAssemblyGraphCellNode* B, const int32 KeyB)
	{
		A->LinkJunction(KeyA, B);
		B->LinkJunction(KeyB, A);
	}
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_MinimumNodeDistance_NullInputDataNeverMatches,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::MinimumNodeDistance::NullInputDataNeverMatches",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A null candidate pointer can never be "too close" and must report no match.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDistanceHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = AddNode(Graph, Cell);

	CHECK_FALSE_MESSAGE(TEXT("Searching for null input data must never report a match."),
		Node->SearchForMatchingCellInputData(nullptr, 5))
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_MinimumNodeDistance_SelfMatchFoundAtZeroDepth,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::MinimumNodeDistance::SelfMatchFoundAtZeroDepth",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A node built from the candidate's own input data is the distance-0 match. This is the MinimumNodeDistance=1
	// case (MaxDepth 0): a cell may not be placed directly adjacent to another instance of the same cell.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDistanceHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = AddNode(Graph, Cell);

	CHECK_MESSAGE(TEXT("A node must match its own input data even at MaxDepth 0."),
		Node->SearchForMatchingCellInputData(&Cell, 0))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_MinimumNodeDistance_FindsMatchWithinDepth,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::MinimumNodeDistance::FindsMatchWithinDepth",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Across a four-node chain of distinct cell types, a search from one end reaches the far type once MaxDepth is
	// large enough to cover the three hops between them.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDistanceHarness;

	FNVirtualCellData CellT1 = MakeCell();
	FNVirtualCellData CellT2 = MakeCell();
	FNVirtualCellData CellT3 = MakeCell();
	FNVirtualCellData CellT4 = MakeCell();

	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellT1);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellT2);
	FNAssemblyGraphCellNode* C = AddNode(Graph, CellT3);
	FNAssemblyGraphCellNode* D = AddNode(Graph, CellT4);
	Link(A, 1, B, 0);
	Link(B, 1, C, 0);
	Link(C, 1, D, 0);

	CHECK_MESSAGE(TEXT("A three-hop-distant type must be found when MaxDepth covers the distance."),
		A->SearchForMatchingCellInputData(&CellT4, 3))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_MinimumNodeDistance_RespectsMaxDepth,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::MinimumNodeDistance::RespectsMaxDepth",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The same four-node chain: a MaxDepth short of the distance must NOT find the far type, proving the search
	// stops expanding at the limit rather than sweeping the whole graph.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDistanceHarness;

	FNVirtualCellData CellT1 = MakeCell();
	FNVirtualCellData CellT2 = MakeCell();
	FNVirtualCellData CellT3 = MakeCell();
	FNVirtualCellData CellT4 = MakeCell();

	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellT1);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellT2);
	FNAssemblyGraphCellNode* C = AddNode(Graph, CellT3);
	FNAssemblyGraphCellNode* D = AddNode(Graph, CellT4);
	Link(A, 1, B, 0);
	Link(B, 1, C, 0);
	Link(C, 1, D, 0);

	CHECK_FALSE_MESSAGE(TEXT("A three-hop-distant type must NOT be found when MaxDepth stops one hop short."),
		A->SearchForMatchingCellInputData(&CellT4, 2))
}

N_TEST_MEDIUM(FNAssemblyGraphCellNodeTests_MinimumNodeDistance_AbsentTypeNeverMatches,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::MinimumNodeDistance::AbsentTypeNeverMatches",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A candidate type that no placed node was built from is never found, however generous the depth budget.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDistanceHarness;

	FNVirtualCellData CellT1 = MakeCell();
	FNVirtualCellData CellT2 = MakeCell();
	FNVirtualCellData CellUnused = MakeCell();

	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellT1);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellT2);
	Link(A, 1, B, 0);

	CHECK_FALSE_MESSAGE(TEXT("A type no node was built from must never be found."),
		A->SearchForMatchingCellInputData(&CellUnused, 16))
}

#endif //WITH_TESTS
