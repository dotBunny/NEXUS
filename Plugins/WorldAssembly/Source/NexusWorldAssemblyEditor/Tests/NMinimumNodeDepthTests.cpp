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

namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDepthHarness
{
	/** Build a cell with a single junction so the node factory has something to copy. */
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.Junctions.Add(0, FNCellJunctionDetails());
		return Cell;
	}

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/**
	 * Mirror of the depth gate in FNVirtualOrganContext::FilterCellInputData. SourceNodeDepth is the depth of
	 * the node the filter is stepping away from (Filter.NodeDepth in production). Kept in lockstep with that
	 * comparison on purpose — if one changes, the other must too.
	 */
	static bool IsGatedOut(const int32 MinimumNodeDepth, const int32 SourceNodeDepth)
	{
		return MinimumNodeDepth > 0 && MinimumNodeDepth > SourceNodeDepth;
	}
}

N_TEST_CRITICAL(FNMinimumNodeDepthTests_DepthRootedAtBone,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MinimumNodeDepth::DepthRootedAtBone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the depth baseline the gate relies on: graph depth is rooted at the bone, so the start cell is
	// NodeDepth 1 (hop 0 from the start), and each Connect adds exactly one. This is the offset that cancels
	// the source-vs-candidate skew in the gate.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDepthHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = FNAssemblyGraphNodeFactory::CreateCellNode(&Cell, FVector::ZeroVector, FRotator::ZeroRotator, FVector(100.f));
	Graph.RegisterNode(Start);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* NeighborA = FNAssemblyGraphNodeFactory::CreateCellNode(&Cell, FVector::ZeroVector, FRotator::ZeroRotator, FVector(100.f));
	Graph.RegisterNode(NeighborA);
	Start->Connect(NeighborA);

	FNAssemblyGraphCellNode* NeighborB = FNAssemblyGraphNodeFactory::CreateCellNode(&Cell, FVector::ZeroVector, FRotator::ZeroRotator, FVector(100.f));
	Graph.RegisterNode(NeighborB);
	NeighborA->Connect(NeighborB);

	CHECK_EQUALS("Bone (graph root) should be NodeDepth 0.", Bone->GetNodeDepth(), 0)
	CHECK_EQUALS("Start cell should be NodeDepth 1 (hop 0 from the start).", Start->GetNodeDepth(), 1)
	CHECK_EQUALS("Start's neighbour should be NodeDepth 2 (hop 1 from the start).", NeighborA->GetNodeDepth(), 2)
	CHECK_EQUALS("Two hops out should be NodeDepth 3 (hop 2 from the start).", NeighborB->GetNodeDepth(), 3)
}

N_TEST_HIGH(FNMinimumNodeDepthTests_FirstEligibleAtHopN,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::MinimumNodeDepth::FirstEligibleAtHopN",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the gate resolves to "hops from the start cell": a cell with MinimumNodeDepth = N is first
	// eligible exactly N hops out, using the real depths produced by Connect as the gate's source depth.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNMinimumNodeDepthHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphCellNode* Start = FNAssemblyGraphNodeFactory::CreateCellNode(&Cell, FVector::ZeroVector, FRotator::ZeroRotator, FVector(100.f));
	Graph.RegisterNode(Start);
	Bone->Connect(Start);

	FNAssemblyGraphCellNode* NeighborA = FNAssemblyGraphNodeFactory::CreateCellNode(&Cell, FVector::ZeroVector, FRotator::ZeroRotator, FVector(100.f));
	Graph.RegisterNode(NeighborA);
	Start->Connect(NeighborA);

	// MinimumNodeDepth = 2 should first appear two hops from the start. Stepping away from the start cell
	// (depth 1) lands a candidate at hop 1, which must be gated out; stepping away from a hop-1 neighbour
	// (depth 2) lands it at hop 2, which must be allowed.
	CHECK_MESSAGE(TEXT("MinimumNodeDepth=2 must be gated out one hop from the start."),
		IsGatedOut(2, Start->GetNodeDepth()))
	CHECK_FALSE_MESSAGE(TEXT("MinimumNodeDepth=2 must be allowed two hops from the start."),
		IsGatedOut(2, NeighborA->GetNodeDepth()))

	// MinimumNodeDepth = 1 may sit adjacent to the start (hop 1) but must never be the start cell itself.
	// Start selection uses a source depth of 0 (the bone pre-filter), which must gate it out.
	CHECK_MESSAGE(TEXT("MinimumNodeDepth=1 must be barred from being the start cell (source depth 0)."),
		IsGatedOut(1, 0))
	CHECK_FALSE_MESSAGE(TEXT("MinimumNodeDepth=1 must be allowed one hop from the start."),
		IsGatedOut(1, Start->GetNodeDepth()))

	// MinimumNodeDepth = 0 is the "no constraint" sentinel and must never gate anything, including the start.
	CHECK_FALSE_MESSAGE(TEXT("MinimumNodeDepth=0 must never gate, even at source depth 0."),
		IsGatedOut(0, 0))
}

#endif //WITH_TESTS
