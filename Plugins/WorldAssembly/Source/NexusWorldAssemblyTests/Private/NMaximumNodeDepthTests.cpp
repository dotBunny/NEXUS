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

namespace NEXUS::UnitTests::NWorldAssembly::FNMaximumNodeDepthHarness
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

N_TEST_HIGH(FNMaximumNodeDepthTests_LastEligibleAtDepthN,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MaximumNodeDepth::LastEligibleAtDepthN",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the 1-based depth gate: MaximumNodeDepth = N is last eligible at NodeDepth N (start cell = 1).
	// The gate takes the candidate's prospective NodeDepth, driven here against the real depths produced by Connect,
	// so a change to FNVirtualOrganContext::IsGatedByMaximumNodeDepth surfaces here.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMaximumNodeDepthHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(Start);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* NeighborA = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
	Graph.RegisterNode(NeighborA);
	Start->Connect(NeighborA);

	// MaximumNodeDepth = 2 is last eligible at NodeDepth 2: a candidate at depth 1 or 2 is allowed, but one at
	// depth 3 must be gated out.
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=2 must be allowed as the start cell (depth 1)."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(2, Start->GetNodeDepth()))
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=2 must be allowed at depth 2."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(2, NeighborA->GetNodeDepth()))
	CHECK_MESSAGE(TEXT("MaximumNodeDepth=2 must be gated at depth 3."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(2, NeighborA->GetNodeDepth() + 1))

	// MaximumNodeDepth = 1 restricts the cell to the start cell (depth 1); anything deeper is gated.
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=1 must be allowed as the start cell (depth 1)."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(1, Start->GetNodeDepth()))
	CHECK_MESSAGE(TEXT("MaximumNodeDepth=1 must be gated at depth 2."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(1, NeighborA->GetNodeDepth()))
}

N_TEST_HIGH(FNMaximumNodeDepthTests_DisabledSentinelNeverGates,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MaximumNodeDepth::DisabledSentinelNeverGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// MaximumNodeDepth = 0 is the "no constraint" sentinel and must never gate anything, at any depth.
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=0 must never gate at the start cell (depth 1)."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(0, 1))
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=0 must never gate deep in the graph."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(0, 100))
}

#endif //WITH_TESTS
