// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness
{
	/** Build a cell with JunctionCount sequentially-keyed (0..N-1) junctions and a deterministic box hull. */
	static FNVirtualCellData MakeCell(const int32 JunctionCount)
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		for (int32 i = 0; i < JunctionCount; i++)
		{
			FNCellJunctionDetails Junction;
			Junction.InstanceIdentifier = i;
			Cell.Junctions.Add(i, Junction);
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

	/** @return The generated link details for the given junction instance, or nullptr when absent. */
	static const FNCellLinkDetails* FindLink(const FNAssemblyGraphCellNode* Node, const int32 JunctionInstanceIdentifier)
	{
		for (const FNCellLinkDetails& Details : Node->GetLinkDetails())
		{
			if (Details.JunctionInstanceIdentifier == JunctionInstanceIdentifier)
			{
				return &Details;
			}
		}
		return nullptr;
	}
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_ConnectorLinks_ConnectorLinkIsReportedConnectedAndFlagged,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::ConnectorLinkIsReportedConnectedAndFlagged",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A connector pairing has to read as connected — that is what stops the junction being capped off at begin play —
	// while still being distinguishable from a direct cell mating, since nothing occupies the opening until the
	// connector actor spawns.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(2);
	FNVirtualCellData CellB = MakeCell(2);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	constexpr int32 ConnectorIdentifier = 7;
	A->LinkJunctionConnector(0, B, ConnectorIdentifier);
	B->LinkJunctionConnector(1, A, ConnectorIdentifier);

	A->GenerateLinkDetails();
	B->GenerateLinkDetails();

	const FNCellLinkDetails* LinkA = FindLink(A, 0);
	const FNCellLinkDetails* LinkB = FindLink(B, 1);
	if (LinkA == nullptr || LinkB == nullptr)
	{
		ADD_ERROR("Expected link details for both paired junctions.");
		return;
	}

	CHECK_MESSAGE(TEXT("A connector-paired junction must report as connected so it is not filled."), LinkA->bConnected)
	CHECK_MESSAGE(TEXT("A connector-paired junction must be flagged as connector-paired."), LinkA->bConnector)
	CHECK_EQUALS("Both ends of a pairing must carry the same connector identifier.", LinkA->ConnectorIdentifier, ConnectorIdentifier)
	CHECK_EQUALS("Both ends of a pairing must carry the same connector identifier.", LinkB->ConnectorIdentifier, ConnectorIdentifier)

	// The junction that was left alone must be untouched by any of this.
	const FNCellLinkDetails* UnpairedLink = FindLink(A, 1);
	if (UnpairedLink == nullptr)
	{
		ADD_ERROR("Expected link details for the unpaired junction.");
		return;
	}
	CHECK_FALSE_MESSAGE(TEXT("An unpaired junction must not report as connected."), UnpairedLink->bConnected)
	CHECK_FALSE_MESSAGE(TEXT("An unpaired junction must not be flagged as connector-paired."), UnpairedLink->bConnector)
	CHECK_EQUALS("An unpaired junction must carry no connector identifier.", UnpairedLink->ConnectorIdentifier, static_cast<int32>(INDEX_NONE))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_ConnectorLinks_DirectMatingIsNotFlaggedAsAConnector,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::DirectMatingIsNotFlaggedAsAConnector",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two cells mated directly by the graph builder are connected but need no connector geometry; only the pairing
	// pass sets the flag, and confusing the two would spawn a connector into a doorway that is already closed.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(1);
	FNVirtualCellData CellB = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	A->LinkJunction(0, B);
	B->LinkJunction(0, A);
	A->GenerateLinkDetails();

	const FNCellLinkDetails* Link = FindLink(A, 0);
	if (Link == nullptr)
	{
		ADD_ERROR("Expected link details for the mated junction.");
		return;
	}

	CHECK_MESSAGE(TEXT("A directly mated junction must report as connected."), Link->bConnected)
	CHECK_FALSE_MESSAGE(TEXT("A directly mated junction must not be flagged as connector-paired."), Link->bConnector)
	CHECK_EQUALS("A directly mated junction must carry no connector identifier.", Link->ConnectorIdentifier, static_cast<int32>(INDEX_NONE))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_ConnectorLinks_UnlinkingClearsTheConnectorRecord,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::UnlinkingClearsTheConnectorRecord",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Returning a junction to the free pool has to drop the pairing with it, or a later link on the same key would
	// inherit a stale connector identifier.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(1);
	FNVirtualCellData CellB = MakeCell(1);
	FNVirtualCellData CellC = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);
	FNAssemblyGraphCellNode* C = AddNode(Graph, CellC);

	A->LinkJunctionConnector(0, B, 3);
	A->UnlinkJunction(0);
	A->LinkJunction(0, C);
	A->GenerateLinkDetails();

	const FNCellLinkDetails* Link = FindLink(A, 0);
	if (Link == nullptr)
	{
		ADD_ERROR("Expected link details for the relinked junction.");
		return;
	}

	CHECK_FALSE_MESSAGE(TEXT("A junction relinked after being unlinked must not inherit the old connector flag."), Link->bConnector)
	CHECK_EQUALS("A junction relinked after being unlinked must not inherit the old connector identifier.",
		Link->ConnectorIdentifier, static_cast<int32>(INDEX_NONE))
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_ConnectorLinks_ExistingLinkIsVisibleFromBothCells,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::ExistingLinkIsVisibleFromBothCells",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The connector pass asks "are these two cells already joined?" to enforce Allow Multiple Cell Connections, and
	// answers it with FindJunctionKeyLinkedTo. A directly mated pair must answer yes from either end, and two
	// unrelated cells must answer no — otherwise the gate either blocks nothing or blocks everything.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(2);
	FNVirtualCellData CellB = MakeCell(2);
	FNVirtualCellData CellC = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);
	FNAssemblyGraphCellNode* C = AddNode(Graph, CellC);

	CHECK_EQUALS("Two unlinked cells must report no link between them.",
		A->FindJunctionKeyLinkedTo(B), static_cast<int32>(INDEX_NONE))

	A->LinkJunction(0, B);
	B->LinkJunction(0, A);

	CHECK_EQUALS("A mated cell must report the junction it linked through.", A->FindJunctionKeyLinkedTo(B), 0)
	CHECK_EQUALS("The far end of a mating must report it too.", B->FindJunctionKeyLinkedTo(A), 0)

	// A third cell sharing no link must stay connectable, so the gate does not spread to unrelated pairs.
	CHECK_EQUALS("An unrelated cell must not report a link.", A->FindJunctionKeyLinkedTo(C), static_cast<int32>(INDEX_NONE))
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_ConnectorLinks_ConnectorPairingBlocksASecondConnection,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::ConnectorPairingBlocksASecondConnection",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The case a candidate-list filter would miss: the link is created part-way through the greedy walk, by the pass
	// itself. A pairing accepted on one set of junctions has to make the same two cells report as joined, so a later
	// candidate between their other junctions is rejected.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(2);
	FNVirtualCellData CellB = MakeCell(2);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	CHECK_EQUALS("The cells must start unjoined.", A->FindJunctionKeyLinkedTo(B), static_cast<int32>(INDEX_NONE))

	A->LinkJunctionConnector(0, B, 1);
	B->LinkJunctionConnector(0, A, 1);

	CHECK_EQUALS("A connector pairing must make the cells report as joined.", A->FindJunctionKeyLinkedTo(B), 0)
	CHECK_EQUALS("The far end of a connector pairing must report it too.", B->FindJunctionKeyLinkedTo(A), 0)

	// Unwinding the pairing frees the cells to connect again, so a rolled-back acceptance leaves no phantom block.
	A->UnlinkJunction(0);
	B->UnlinkJunction(0);
	CHECK_EQUALS("Unlinking must return the cells to reporting no link.",
		A->FindJunctionKeyLinkedTo(B), static_cast<int32>(INDEX_NONE))
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_ConnectorLinks_IndirectAndNonCellLinksDoNotBlock,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::IndirectAndNonCellLinksDoNotBlock",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Only a direct link between the two cells blocks. Two cells at either end of a chain are exactly the pair a
	// connector is most useful for, and a cell linked to a bone must not read as linked to some other cell.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(2);
	FNVirtualCellData CellB = MakeCell(2);
	FNVirtualCellData CellMiddle = MakeCell(2);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);
	FNAssemblyGraphCellNode* Middle = AddNode(Graph, CellMiddle);

	// A - Middle - B, with nothing joining the two ends.
	A->LinkJunction(0, Middle);
	Middle->LinkJunction(0, A);
	Middle->LinkJunction(1, B);
	B->LinkJunction(0, Middle);

	CHECK_EQUALS("Cells joined only through a third must report no direct link.",
		A->FindJunctionKeyLinkedTo(B), static_cast<int32>(INDEX_NONE))
	CHECK_EQUALS("The far end of an indirect chain must agree.",
		B->FindJunctionKeyLinkedTo(A), static_cast<int32>(INDEX_NONE))

	// A junction linked to the graph's root bone must not be mistaken for a link to another cell.
	FNVirtualCellData CellBone = MakeCell(1);
	FNAssemblyGraphCellNode* BoneLinked = AddNode(Graph, CellBone);
	BoneLinked->LinkJunction(0, const_cast<FNAssemblyGraphNode*>(Graph.GetRootNode()));

	CHECK_EQUALS("A cell linked to a bone must not report a link to an unrelated cell.",
		BoneLinked->FindJunctionKeyLinkedTo(A), static_cast<int32>(INDEX_NONE))
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_ConnectorLinks_ConnectorEdgeMakesCellsReachable,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::ConnectorLinks::ConnectorEdgeMakesCellsReachable",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Hot pathing walks upstream/downstream node links, not the junction map, so a pairing only shows up in it if the
	// node-level edge is wired too. Without that a connector players can walk through would be routed around.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectorLinksHarness;

	FNVirtualCellData CellA = MakeCell(1);
	FNVirtualCellData CellB = MakeCell(1);
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);

	CHECK_MESSAGE(TEXT("Two unconnected cells must start with no node-level edge."),
		A->GetDownstreamNodes().IsEmpty() && B->GetUpstreamNodes().IsEmpty())

	A->LinkJunctionConnector(0, B, 1);
	B->LinkJunctionConnector(0, A, 1);

	CHECK_MESSAGE(TEXT("Linking junctions alone must not wire the node-level edge; the pair owns that call."),
		A->GetDownstreamNodes().IsEmpty())

	A->Connect(B);

	CHECK_MESSAGE(TEXT("Connecting the pair must make the far cell reachable downstream."),
		A->GetDownstreamNodes().Contains(B))
	CHECK_MESSAGE(TEXT("Connecting the pair must make the near cell reachable upstream."),
		B->GetUpstreamNodes().Contains(A))
}

#endif //WITH_TESTS
