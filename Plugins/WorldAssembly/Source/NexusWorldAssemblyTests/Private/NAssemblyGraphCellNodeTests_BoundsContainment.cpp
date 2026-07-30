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

namespace NEXUS::UnitTests::NWorldAssembly::FNBoundsContainmentHarness
{
	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	/** A cell whose author-space bounds AND hull are both the axis-aligned box [Min, Max]. */
	static FNVirtualCellData MakeBoxCell(const FVector& Min, const FVector& Max)
	{
		FNVirtualCellData Cell;
		const FBox Box(Min, Max);
		Cell.CellDetails.Bounds = Box;
		// A full convex box hull (faces + triangulation), matching the shape production hulls arrive in, so the
		// cell-node constructor's IsConvex()/EnsureCachedFacePlanes() path runs cleanly instead of warning on a
		// vertex-only hull. The 8 hull vertices are still exactly the box corners the containment checks expect.
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Box);
		return Cell;
	}

	/** Create a cell node from Cell at the given world placement and register it with Graph (which then owns it). */
	static FNAssemblyGraphCellNode* PlaceCell(FNAssemblyGraph& Graph, FNVirtualCellData& Cell,
		const FVector& WorldPosition, const FRotator& WorldRotation = FRotator::ZeroRotator)
	{
		FNAssemblyGraphNodeParams Params;
		Params.WorldPosition = WorldPosition;
		Params.WorldRotation = WorldRotation;
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_BoundsContainment_CellAtOriginIsInside,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::BoundsContainment::CellAtOriginIsInside",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A small cell sitting at the organ origin is comfortably inside the organ bounds on both the AABB and the
	// hull test. This also exercises the identity-transform path through the hull bake (no rotation, no offset).
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBoundsContainmentHarness;

	const FBox Organ(FVector(-100.0), FVector(100.0));
	FNVirtualCellData Cell = MakeBoxCell(FVector(-50.0), FVector(50.0));
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = PlaceCell(Graph, Cell, FVector::ZeroVector);

	CHECK_MESSAGE(TEXT("A centered cell's AABB must read as inside the organ bounds."), Node->IsBoundsInside(Organ))
	CHECK_MESSAGE(TEXT("A centered cell's hull must read as inside the organ bounds."), Node->IsHullInside(Organ))
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_BoundsContainment_CellPlacedOutsideIsRejected,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::BoundsContainment::CellPlacedOutsideIsRejected",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The decisive bake check: a cell whose author-space box is small enough to fit the organ, but which is
	// PLACED far outside it, must read as outside on both tests. If the hull were left in author space rather
	// than baked into world space at construction, IsHullInside would wrongly report the cell as inside.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBoundsContainmentHarness;

	const FBox Organ(FVector(-100.0), FVector(100.0));
	FNVirtualCellData Cell = MakeBoxCell(FVector(-50.0), FVector(50.0));
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = PlaceCell(Graph, Cell, FVector(200.0, 0.0, 0.0));

	CHECK_FALSE_MESSAGE(TEXT("A cell placed well outside the organ must not read as bounds-inside."), Node->IsBoundsInside(Organ))
	CHECK_FALSE_MESSAGE(TEXT("A cell placed well outside the organ must not read as hull-inside (proves the hull is baked to world space)."),
		Node->IsHullInside(Organ))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_BoundsContainment_StraddlingBoundaryIsRejected,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::BoundsContainment::StraddlingBoundaryIsRejected",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell placed so its world box pokes past the +X face (world span 10..110 against a 100 limit) must be
	// rejected: the AABB is not inside, and the hull check (its own decisive test) also fails.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBoundsContainmentHarness;

	const FBox Organ(FVector(-100.0), FVector(100.0));
	FNVirtualCellData Cell = MakeBoxCell(FVector(-50.0), FVector(50.0));
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = PlaceCell(Graph, Cell, FVector(60.0, 0.0, 0.0));

	CHECK_FALSE_MESSAGE(TEXT("A cell straddling the +X face must not read as bounds-inside."), Node->IsBoundsInside(Organ))
	CHECK_FALSE_MESSAGE(TEXT("A cell straddling the +X face must not read as hull-inside."), Node->IsHullInside(Organ))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_BoundsContainment_HullBakedIntoWorldSpace,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::BoundsContainment::HullBakedIntoWorldSpace",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Pins the world-space hull bake from both directions for one placement: a cell at (200,0,0) must be OUTSIDE
	// an organ at the origin, yet INSIDE an organ shifted to enclose that world position. A non-baked hull would
	// flip the first; a double-baked hull (transform applied twice) would flip the second.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBoundsContainmentHarness;

	FNVirtualCellData Cell = MakeBoxCell(FVector(-50.0), FVector(50.0));
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = PlaceCell(Graph, Cell, FVector(200.0, 0.0, 0.0));

	const FBox OrganAtOrigin(FVector(-100.0), FVector(100.0));
	const FBox OrganAroundCell(FVector(100.0, -100.0, -100.0), FVector(300.0, 100.0, 100.0));

	CHECK_FALSE_MESSAGE(TEXT("The world-space hull (centered at 200) must be outside an organ at the origin."),
		Node->IsHullInside(OrganAtOrigin))
	CHECK_MESSAGE(TEXT("The world-space hull (centered at 200) must be inside an organ that encloses 100..300 on X."),
		Node->IsHullInside(OrganAroundCell))
}

N_TEST_MEDIUM(FNAssemblyGraphCellNodeTests_BoundsContainment_FlushToBoundaryIsRejected,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::BoundsContainment::FlushToBoundaryIsRejected",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Documents a sharp edge: FBox::IsInside is EXCLUSIVE, so a cell whose far face sits EXACTLY on the organ
	// boundary is treated as outside and discarded. A [-50,50] cell placed at (50,0,0) spans X 0..100 against an
	// organ whose Max.X is exactly 100, and is rejected on both tests. This is inconsistent with
	// FNBoundsUtils::IsPointInBounds, which uses the inclusive IsInsideOrOn — see the report note. If the
	// containment is changed to inclusive, both assertions below must flip to expect "inside".
	using namespace NEXUS::UnitTests::NWorldAssembly::FNBoundsContainmentHarness;

	const FBox Organ(FVector(-100.0), FVector(100.0));
	FNVirtualCellData Cell = MakeBoxCell(FVector(-50.0), FVector(50.0));
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = PlaceCell(Graph, Cell, FVector(50.0, 0.0, 0.0));

	CHECK_FALSE_MESSAGE(TEXT("A cell flush to the +X boundary is currently rejected by the exclusive AABB test."),
		Node->IsBoundsInside(Organ))
	CHECK_FALSE_MESSAGE(TEXT("A cell flush to the +X boundary is currently rejected by the exclusive hull test."),
		Node->IsHullInside(Organ))
}

#endif //WITH_TESTS
