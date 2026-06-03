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

namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphHarness
{
	/** Build a cell with JunctionCount sequentially-keyed junctions and deterministic bounds. */
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

	/** A bone root so the graph has a valid (non-cell, uncounted) starting node. */
	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

N_TEST_CRITICAL(FNAssemblyGraphTests_CellInstanceCount_CountsOncePerInstance,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellInstanceCount::CountsOncePerInstance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies UsedCount tracks placed instances, not junction links: a single multi-junction cell
	// counts once, and linking its junctions never changes the count.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphHarness;

	FNVirtualCellData Cell = MakeCell(3);
	FNAssemblyGraphBoneNode* BoneRoot = MakeBoneRoot();
	FNAssemblyGraph Graph(BoneRoot, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	CHECK_EQUALS("UsedCount should start at 0.", Cell.UsedCount, 0)

	FNAssemblyGraphCellNode* NodeA = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NodeA);
	CHECK_EQUALS("Registering one cell node should increment UsedCount to 1.", Cell.UsedCount, 1)

	// Linking three junctions on the same instance must not change the count.
	NodeA->LinkJunction(0, BoneRoot);
	NodeA->LinkJunction(1, BoneRoot);
	NodeA->LinkJunction(2, BoneRoot);
	CHECK_EQUALS("Linking junctions must not change UsedCount.", Cell.UsedCount, 1)

	FNAssemblyGraphCellNode* NodeB = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NodeB);
	CHECK_EQUALS("A second instance should increment UsedCount to 2.", Cell.UsedCount, 2)

	Graph.UnregisterNode(NodeB);
	CHECK_EQUALS("Unregistering an instance should decrement UsedCount to 1.", Cell.UsedCount, 1)
	delete NodeB;
}

N_TEST_HIGH(FNAssemblyGraphTests_CellInstanceCount_MaximumCountByInstance,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellInstanceCount::MaximumCountByInstance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies MaximumCount is exhausted by placed instances, not junction links — a multi-junction
	// first placement must not consume a MaximumCount of 2.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphHarness;

	FNVirtualCellData Cell = MakeCell(2);
	Cell.MaximumCount = 2;
	FNAssemblyGraphBoneNode* BoneRoot = MakeBoneRoot();
	FNAssemblyGraph Graph(BoneRoot, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* NodeA = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NodeA);
	NodeA->LinkJunction(0, BoneRoot);
	NodeA->LinkJunction(1, BoneRoot);
	CHECK_MESSAGE(TEXT("One instance of two-junction cell should still be selectable when MaximumCount is 2."), Cell.IsValidSelection())

	FNAssemblyGraphCellNode* NodeB = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NodeB);
	CHECK_FALSE_MESSAGE(TEXT("Two instances should reach MaximumCount and become unselectable."), Cell.IsValidSelection())

	Graph.UnregisterNode(NodeB);
	delete NodeB;
	CHECK_MESSAGE(TEXT("Removing an instance should make the cell selectable again."), Cell.IsValidSelection())
}

N_TEST_HIGH(FNAssemblyGraphTests_CellInstanceCount_UniqueByInstance,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellInstanceCount::UniqueByInstance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies a unique cell is consumed by exactly one instance even when connected through several junctions.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphHarness;

	FNVirtualCellData Cell = MakeCell(3);
	Cell.MinimumCount = 1;
	Cell.MaximumCount = 1;
	CHECK_MESSAGE(TEXT("Cell should report as unique."), Cell.IsUnique())

	FNAssemblyGraphBoneNode* BoneRoot = MakeBoneRoot();
	FNAssemblyGraph Graph(BoneRoot, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	CHECK_MESSAGE(TEXT("Unused unique cell should be selectable."), Cell.IsValidSelection())

	FNAssemblyGraphCellNode* NodeA = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NodeA);
	NodeA->LinkJunction(0, BoneRoot);
	NodeA->LinkJunction(1, BoneRoot);
	NodeA->LinkJunction(2, BoneRoot);

	CHECK_EQUALS("A single unique instance must count exactly once regardless of junction links.", Cell.UsedCount, 1)
	CHECK_FALSE_MESSAGE(TEXT("A placed unique cell should no longer be selectable."), Cell.IsValidSelection())
}

#endif //WITH_TESTS
