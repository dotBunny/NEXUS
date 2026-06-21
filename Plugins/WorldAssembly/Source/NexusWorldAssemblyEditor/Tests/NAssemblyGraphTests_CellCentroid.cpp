// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNCellCentroidHarness
{
	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		// A Bone root keeps the graph's cell count (and thus the centroid) empty until cells are registered.
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/**
	 * Create a cell node at Position from the shared Cell and register it with Graph (which then owns and frees it).
	 * The default FNVirtualCellData has zero-size bounds, so its world bounds center coincides with Position unless
	 * the caller sets CellDetails.Bounds first.
	 */
	static FNAssemblyGraphCellNode* AddCellAt(FNAssemblyGraph& Graph, FNVirtualCellData& Cell, const FVector& Position)
	{
		FNAssemblyGraphNodeParams Params;
		Params.WorldPosition = Position;
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}
}

N_TEST_HIGH(FNAssemblyGraphTests_CellCentroid_EmptyGraphReturnsZero,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellCentroid::EmptyGraphReturnsZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With only a Bone root (no cell nodes), the centroid is undefined and reports ZeroVector.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellCentroidHarness;

	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	CHECK_MESSAGE(TEXT("A graph with no cell nodes must report a ZeroVector centroid."),
		Graph.GetCellCentroid().Equals(FVector::ZeroVector))
}

N_TEST_CRITICAL(FNAssemblyGraphTests_CellCentroid_MeanOfPlacedCells,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellCentroid::MeanOfPlacedCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The centroid is the mean of every registered cell node's world bounds center. These cells have zero-size
	// bounds, so each contributes its placement position.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellCentroidHarness;

	FNVirtualCellData Cell;
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	AddCellAt(Graph, Cell, FVector(0.0, 0.0, 0.0));
	AddCellAt(Graph, Cell, FVector(300.0, 0.0, 0.0));
	AddCellAt(Graph, Cell, FVector(0.0, 300.0, 0.0));

	CHECK_MESSAGE(TEXT("The centroid of cells at (0,0,0), (300,0,0) and (0,300,0) must be (100,100,0)."),
		Graph.GetCellCentroid().Equals(FVector(100.0, 100.0, 0.0)))
}

N_TEST_HIGH(FNAssemblyGraphTests_CellCentroid_UsesCellBoundsCenter,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellCentroid::UsesCellBoundsCenter",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The centroid uses each cell's world bounds center, not its pivot. A cell whose authored bounds sit off its
	// pivot contributes that offset: local bounds [(0,0,0)..(200,0,0)] center at (100,0,0), placed at the origin,
	// reads as (100,0,0) rather than the pivot (0,0,0).
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellCentroidHarness;

	FNVirtualCellData Cell;
	Cell.CellDetails.Bounds = FBox(FVector(0.0, 0.0, 0.0), FVector(200.0, 0.0, 0.0));

	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	AddCellAt(Graph, Cell, FVector::ZeroVector);

	CHECK_MESSAGE(TEXT("The centroid must use the cell's bounds center (100,0,0), not its pivot (0,0,0)."),
		Graph.GetCellCentroid().Equals(FVector(100.0, 0.0, 0.0)))
}

N_TEST_HIGH(FNAssemblyGraphTests_CellCentroid_UpdatesAfterUnregister,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::CellCentroid::UpdatesAfterUnregister",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Removing a cell subtracts exactly its contribution, so the centroid reflects only the cells that remain.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellCentroidHarness;

	FNVirtualCellData Cell;
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	AddCellAt(Graph, Cell, FVector(0.0, 0.0, 0.0));
	AddCellAt(Graph, Cell, FVector(300.0, 0.0, 0.0));
	FNAssemblyGraphCellNode* Removable = AddCellAt(Graph, Cell, FVector(0.0, 300.0, 0.0));

	// UnregisterNode hands ownership back to the caller, so the test must free the detached node itself.
	Graph.UnregisterNode(Removable);
	delete Removable;

	CHECK_MESSAGE(TEXT("After removing the (0,300,0) cell, the centroid of the remaining two must be (150,0,0)."),
		Graph.GetCellCentroid().Equals(FVector(150.0, 0.0, 0.0)))
}

#endif //WITH_TESTS
