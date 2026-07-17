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

namespace NEXUS::UnitTests::NWorldAssembly::FNCellNodeJunctionsHarness
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

	/** Create a cell node from Cell and register it with Graph (which then owns and frees it). */
	static FNAssemblyGraphCellNode* AddNode(FNAssemblyGraph& Graph, FNVirtualCellData& Cell)
	{
		FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(FNAssemblyGraphNodeParams(), &Cell, FVector(100.f));
		Graph.RegisterNode(Node);
		return Node;
	}
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_Junctions_FreshCellHasAllJunctionsOpen,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::Junctions::FreshCellHasAllJunctionsOpen",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A newly placed cell starts with every authored junction free.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellNodeJunctionsHarness;

	FNVirtualCellData Cell = MakeCell(3);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = AddNode(Graph, Cell);

	CHECK_MESSAGE(TEXT("A fresh multi-junction cell must report open junctions."), Node->HasOpenJunctions())
	CHECK_EQUALS("Every authored junction must start in the open set.", Node->GetOpenJunctions().Num(), 3)
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_Junctions_LinkRemovesJunctionFromOpenSet,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::Junctions::LinkRemovesJunctionFromOpenSet",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Linking a junction removes exactly that key from the open set and records the linked node; other junctions
	// stay free and report no link.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellNodeJunctionsHarness;

	FNVirtualCellData CellA = MakeCell(3);
	FNVirtualCellData CellB = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	A->LinkJunction(0, B);

	const TMap<int32, FNCellJunctionDetails*> Open = A->GetOpenJunctions();
	CHECK_EQUALS("Linking one junction must leave two of three open.", Open.Num(), 2)
	CHECK_FALSE_MESSAGE(TEXT("The linked junction key must no longer be open."), Open.Contains(0))
	CHECK_MESSAGE(TEXT("The cell must still report open junctions."), A->HasOpenJunctions())

	CHECK_MESSAGE(TEXT("GetLinkedNode must return the node the junction was linked to."), A->GetLinkedNode(0) == B)
	CHECK_MESSAGE(TEXT("An unlinked junction must report no linked node."), A->GetLinkedNode(1) == nullptr)
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_Junctions_UnlinkReturnsJunctionToOpenSet,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::Junctions::UnlinkReturnsJunctionToOpenSet",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Unlinking a junction returns it to the free pool and clears its recorded link — the rollback RemoveCellNode needs.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellNodeJunctionsHarness;

	FNVirtualCellData CellA = MakeCell(3);
	FNVirtualCellData CellB = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	A->LinkJunction(0, B);
	A->UnlinkJunction(0);

	const TMap<int32, FNCellJunctionDetails*> Open = A->GetOpenJunctions();
	CHECK_EQUALS("Unlinking must restore the junction to the open set.", Open.Num(), 3)
	CHECK_MESSAGE(TEXT("The previously linked junction key must be open again."), Open.Contains(0))
	CHECK_MESSAGE(TEXT("The unlinked junction must report no linked node."), A->GetLinkedNode(0) == nullptr)
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_Junctions_FindJunctionKeyLinkedToReturnsKeyOrNone,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::Junctions::FindJunctionKeyLinkedToReturnsKeyOrNone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The reverse lookup finds the junction key linked to a given node, or INDEX_NONE when none links to it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellNodeJunctionsHarness;

	FNVirtualCellData CellA = MakeCell(3);
	FNVirtualCellData CellB = MakeCell(1);
	FNVirtualCellData CellC = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);
	FNAssemblyGraphCellNode* C = AddNode(Graph, CellC);

	A->LinkJunction(1, B);

	CHECK_EQUALS("FindJunctionKeyLinkedTo must return the key linked to B.", A->FindJunctionKeyLinkedTo(B), 1)
	CHECK_EQUALS("A node nothing links to must return INDEX_NONE.", A->FindJunctionKeyLinkedTo(C), INDEX_NONE)
}

N_TEST_MEDIUM(FNAssemblyGraphCellNodeTests_Junctions_AllJunctionsLinkedReportsNoneOpen,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::Junctions::AllJunctionsLinkedReportsNoneOpen",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A cell with every junction linked reports no open junctions and an empty open set.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellNodeJunctionsHarness;

	FNVirtualCellData CellA = MakeCell(2);
	FNVirtualCellData CellB = MakeCell(2);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	A->LinkJunction(0, B);
	A->LinkJunction(1, B);

	CHECK_FALSE_MESSAGE(TEXT("A fully linked cell must report no open junctions."), A->HasOpenJunctions())
	CHECK_EQUALS("A fully linked cell must expose an empty open set.", A->GetOpenJunctions().Num(), 0)
}

#endif //WITH_TESTS
