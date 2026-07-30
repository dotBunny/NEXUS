// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDepthHarness
{
	/** Build a cell with a single junction so the node factory has something to copy. */
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		Cell.Junctions.Add(0, FNCellJunctionDetails());
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

N_TEST_CRITICAL(FNMinimumNodeDepthTests_DepthRootedAtBone,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MinimumNodeDepth::DepthRootedAtBone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the depth baseline the gate relies on: graph depth is rooted at the bone (NodeDepth 0), the start
	// cell is NodeDepth 1, and each Connect adds exactly one. Min/MaxNodeDepth are authored against these values.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDepthHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(Start);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* NeighborA = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NeighborA);
	Start->Connect(NeighborA);

	FNAssemblyGraphCellNode* NeighborB = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NeighborB);
	NeighborA->Connect(NeighborB);

	CHECK_EQUALS("Bone (graph root) should be NodeDepth 0.", Bone->GetNodeDepth(), 0)
	CHECK_EQUALS("Start cell should be NodeDepth 1.", Start->GetNodeDepth(), 1)
	CHECK_EQUALS("Start's neighbour should be NodeDepth 2.", NeighborA->GetNodeDepth(), 2)
	CHECK_EQUALS("Two hops out should be NodeDepth 3.", NeighborB->GetNodeDepth(), 3)
}

N_TEST_HIGH(FNMinimumNodeDepthTests_FirstEligibleAtDepthN,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MinimumNodeDepth::FirstEligibleAtDepthN",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the 1-based depth gate: MinimumNodeDepth = N is first eligible at NodeDepth N (start cell = 1).
	// The gate takes the candidate's prospective NodeDepth, driven here against the real depths produced by Connect,
	// so a change to FNVirtualOrganContext::IsGatedByMinimumNodeDepth surfaces here.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDepthHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(Start);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* NeighborA = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NeighborA);
	Start->Connect(NeighborA);

	// MinimumNodeDepth = 2 is first eligible at NodeDepth 2: a candidate landing as the start cell (depth 1) is
	// gated out, while one landing at depth 2 is allowed.
	CHECK_MESSAGE(TEXT("MinimumNodeDepth=2 must gate a candidate at the start cell (depth 1)."),
		FNVirtualOrganContext::IsGatedByMinimumNodeDepth(2, Start->GetNodeDepth()))
	CHECK_FALSE_MESSAGE(TEXT("MinimumNodeDepth=2 must be allowed at depth 2."),
		FNVirtualOrganContext::IsGatedByMinimumNodeDepth(2, NeighborA->GetNodeDepth()))

	// MinimumNodeDepth = 1 is the start cell itself and beyond, so it never gates.
	CHECK_FALSE_MESSAGE(TEXT("MinimumNodeDepth=1 must be allowed as the start cell (depth 1)."),
		FNVirtualOrganContext::IsGatedByMinimumNodeDepth(1, Start->GetNodeDepth()))

	// MinimumNodeDepth = 0 is the "no constraint" sentinel and must never gate anything, including the start.
	CHECK_FALSE_MESSAGE(TEXT("MinimumNodeDepth=0 must never gate, even at the start cell (depth 1)."),
		FNVirtualOrganContext::IsGatedByMinimumNodeDepth(0, Start->GetNodeDepth()))
}

#endif //WITH_TESTS
