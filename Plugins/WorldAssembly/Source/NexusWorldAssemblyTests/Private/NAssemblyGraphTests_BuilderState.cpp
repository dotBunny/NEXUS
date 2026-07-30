// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphBuilderStateHarness
{
	/** Build a cell with JunctionCount sequentially-keyed (0..N-1) junctions and a deterministic box hull. */
	static FNVirtualCellData MakeCell(const int32 JunctionCount)
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		for (int32 i = 0; i < JunctionCount; i++)
		{
			Cell.Junctions.Add(i, FNCellJunctionDetails());
		}
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	static FNAssemblyGraphCellNode* AddNode(FNAssemblyGraph& Graph, FNVirtualCellData& Cell)
	{
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}
}

N_TEST_HIGH(FNAssemblyGraphTests_BuilderState_GetNodesWithOpenJunctionsReturnsOnlyOpenCells,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::BuilderState::GetNodesWithOpenJunctionsReturnsOnlyOpenCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The expansion frontier is exactly the cell nodes that still have a free junction: a fully linked cell and the
	// (non-cell) bone root are excluded.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphBuilderStateHarness;

	FNVirtualCellData CellA = MakeCell(1); // becomes fully linked
	FNVirtualCellData CellB = MakeCell(2); // keeps one junction free
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	A->LinkJunction(0, B); // A now has no free junctions
	B->LinkJunction(0, A); // B still has key 1 free

	const TArray<FNAssemblyGraphNode*> Open = Graph.GetNodesWithOpenJunctions();
	CHECK_EQUALS("Only the one cell with a free junction should be returned.", Open.Num(), 1)
	CHECK_MESSAGE(TEXT("The cell with a free junction must be in the open-frontier set."), Open.Contains(B))
	CHECK_FALSE_MESSAGE(TEXT("A fully linked cell must not be in the open-frontier set."), Open.Contains(A))
}

N_TEST_MEDIUM(FNAssemblyGraphTests_BuilderState_GetNodesWithOpenJunctionsEmptyWhenAllLinked,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::BuilderState::GetNodesWithOpenJunctionsEmptyWhenAllLinked",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When every cell's junctions are linked, the frontier is empty (BFS expansion would terminate).
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphBuilderStateHarness;

	FNVirtualCellData CellA = MakeCell(1);
	FNVirtualCellData CellB = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	A->LinkJunction(0, B);
	B->LinkJunction(0, A);

	CHECK_EQUALS("No cell has a free junction, so the open-frontier set must be empty.",
		Graph.GetNodesWithOpenJunctions().Num(), 0)
}

N_TEST_HIGH(FNAssemblyGraphTests_BuilderState_CleanupBuilderReferencesNullsCellInputPointers,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::BuilderState::CleanupBuilderReferencesNullsCellInputPointers",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Before hand-off to the spawn stage the builder-only input pointers must be dropped on every cell node so the
	// graph carries no dangling references into builder scratch data.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphBuilderStateHarness;

	FNVirtualCellData CellA = MakeCell(1);
	FNVirtualCellData CellB = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	CHECK_MESSAGE(TEXT("Cell nodes must hold their input pointer before cleanup."),
		A->GetInputDataPtr() != nullptr && B->GetInputDataPtr() != nullptr)

	Graph.CleanupBuilderReferences();

	CHECK_MESSAGE(TEXT("CleanupBuilderReferences must null the input pointer on every cell node."),
		A->GetInputDataPtr() == nullptr && B->GetInputDataPtr() == nullptr)
}

#endif //WITH_TESTS
