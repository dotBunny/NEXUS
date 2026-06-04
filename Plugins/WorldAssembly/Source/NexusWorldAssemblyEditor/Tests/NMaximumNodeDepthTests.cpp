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

N_TEST_HIGH(FNMaximumNodeDepthTests_LastEligibleAtHopN,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MaximumNodeDepth::LastEligibleAtHopN",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the gate resolves to "hops from the start cell": a cell with MaximumNodeDepth = N is last eligible
	// exactly N hops out. Drives the real production gate (FNVirtualOrganContext::IsGatedByMaximumNodeDepth) against
	// the real depths produced by Connect, so a change to the comparison surfaces here. The source depth used to
	// select a candidate equals that candidate's hop-count from the start cell (the same offset cancellation the
	// minimum-depth gate relies on): start selection uses source depth 0, one hop out uses the start's depth (1), etc.
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

	// MaximumNodeDepth = 1 may sit at the start (source depth 0) and one hop out (source depth 1), but a candidate
	// stepping away from a hop-1 neighbour (source depth 2) lands at hop 2 and must be gated out.
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=1 must be allowed as the start cell (source depth 0)."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(1, 0))
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=1 must be allowed one hop from the start."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(1, Start->GetNodeDepth()))
	CHECK_MESSAGE(TEXT("MaximumNodeDepth=1 must be gated two hops from the start."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(1, NeighborA->GetNodeDepth()))

	// MaximumNodeDepth = 0 may only be the start cell (source depth 0); anything past the start is gated.
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=0 must be allowed as the start cell (source depth 0)."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(0, 0))
	CHECK_MESSAGE(TEXT("MaximumNodeDepth=0 must be gated one hop from the start."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(0, Start->GetNodeDepth()))
}

N_TEST_HIGH(FNMaximumNodeDepthTests_DisabledSentinelNeverGates,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MaximumNodeDepth::DisabledSentinelNeverGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// MaximumNodeDepth = -1 is the "no constraint" sentinel and must never gate anything, at any depth.
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=-1 must never gate at the start (source depth 0)."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(-1, 0))
	CHECK_FALSE_MESSAGE(TEXT("MaximumNodeDepth=-1 must never gate deep in the graph."),
		FNVirtualOrganContext::IsGatedByMaximumNodeDepth(-1, 100))
}

#endif //WITH_TESTS
