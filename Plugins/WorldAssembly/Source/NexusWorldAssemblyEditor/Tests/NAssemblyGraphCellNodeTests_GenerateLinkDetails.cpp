// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Cell/NCellLinkDetails.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNGenerateLinkDetailsHarness
{
	// GenerateLinkDetails emits one FNCellLinkDetails per junction in WorldJunctions. These helpers deliberately give
	// each junction an InstanceIdentifier that differs from its map key, so the tests prove the generated data carries
	// the persisted InstanceIdentifier rather than the junction's map key.
	static FNVirtualCellData MakeCellShell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-100.0), FVector(100.0));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		return Cell;
	}

	/** Add a junction under map key Key carrying the persisted InstanceId. */
	static void AddJunction(FNVirtualCellData& Cell, const int32 Key, const int32 InstanceId)
	{
		FNCellJunctionDetails Junction;
		Junction.InstanceIdentifier = InstanceId;
		Cell.Junctions.Add(Key, Junction);
	}

	static FNAssemblyGraphBoneNode* MakeBone()
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

	/** Bidirectionally link two cell nodes through the given junction keys. */
	static void Link(FNAssemblyGraphCellNode* A, const int32 KeyA, FNAssemblyGraphCellNode* B, const int32 KeyB)
	{
		A->LinkJunction(KeyA, B);
		B->LinkJunction(KeyB, A);
	}

	/** Locate the generated entry for the junction with the given InstanceIdentifier, or nullptr. */
	static const FNCellLinkDetails* FindByJunctionInstanceId(const TArray<FNCellLinkDetails>& Details, const int32 InstanceId)
	{
		for (const FNCellLinkDetails& Detail : Details)
		{
			if (Detail.JunctionInstanceIdentifier == InstanceId) return &Detail;
		}
		return nullptr;
	}
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_GenerateLinkDetails_OneEntryPerJunction,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::GenerateLinkDetails::OneEntryPerJunction",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every junction on the cell produces exactly one link entry, keyed by its persisted InstanceIdentifier.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNGenerateLinkDetailsHarness;

	FNVirtualCellData Cell = MakeCellShell();
	AddJunction(Cell, 0, 100);
	AddJunction(Cell, 1, 101);

	FNAssemblyGraph Graph(MakeBone(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = AddNode(Graph, Cell);

	Node->GenerateLinkDetails();

	CHECK_EQUALS("There must be one link entry per junction.", Node->GetLinkDetails().Num(), 2)
	CHECK_MESSAGE(TEXT("An entry must exist for the junction with InstanceIdentifier 100."),
		FindByJunctionInstanceId(Node->GetLinkDetails(), 100) != nullptr)
	CHECK_MESSAGE(TEXT("An entry must exist for the junction with InstanceIdentifier 101."),
		FindByJunctionInstanceId(Node->GetLinkDetails(), 101) != nullptr)
}

N_TEST_HIGH(FNAssemblyGraphCellNodeTests_GenerateLinkDetails_FreeJunctionLeavesConnectionDefault,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::GenerateLinkDetails::FreeJunctionLeavesConnectionDefault",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An unlinked junction is reported as not connected, with both connection identifiers left at their -1 defaults.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNGenerateLinkDetailsHarness;

	FNVirtualCellData Cell = MakeCellShell();
	AddJunction(Cell, 0, 100);

	FNAssemblyGraph Graph(MakeBone(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = AddNode(Graph, Cell);

	Node->GenerateLinkDetails();

	const FNCellLinkDetails* Entry = FindByJunctionInstanceId(Node->GetLinkDetails(), 100);
	if (Entry == nullptr)
	{
		ADD_ERROR("Expected a link entry for the junction with InstanceIdentifier 100.");
		return;
	}

	CHECK_FALSE_MESSAGE(TEXT("A free junction must report bConnected == false."), Entry->bConnected)
	CHECK_EQUALS("A free junction must leave ConnectedNodeIdentifier at -1.", Entry->ConnectedNodeIdentifier, -1)
	CHECK_EQUALS("A free junction must leave ConnectedJunctionInstanceIdentifier at -1.",
		Entry->ConnectedJunctionInstanceIdentifier, -1)
}

N_TEST_CRITICAL(FNAssemblyGraphCellNodeTests_GenerateLinkDetails_LinkedCellRecordsConnection,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::GenerateLinkDetails::LinkedCellRecordsConnection",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A junction linked to another cell records that cell's node identifier and the InstanceIdentifier of the far
	// junction that links back, while an unrelated free junction on the same cell stays at its defaults.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNGenerateLinkDetailsHarness;

	FNVirtualCellData CellA = MakeCellShell();
	AddJunction(CellA, 0, 100); // linked to B
	AddJunction(CellA, 1, 101); // left free

	FNVirtualCellData CellB = MakeCellShell();
	AddJunction(CellB, 0, 200); // links back to A
	AddJunction(CellB, 1, 201);

	FNAssemblyGraph Graph(MakeBone(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphCellNode* B = AddNode(Graph, CellB);
	Link(A, 0, B, 0);

	A->GenerateLinkDetails();

	const FNCellLinkDetails* Linked = FindByJunctionInstanceId(A->GetLinkDetails(), 100);
	if (Linked == nullptr)
	{
		ADD_ERROR("Expected a link entry for the linked junction with InstanceIdentifier 100.");
		return;
	}
	CHECK_MESSAGE(TEXT("A linked junction must report bConnected == true."), Linked->bConnected)
	CHECK_EQUALS("ConnectedNodeIdentifier must match the linked node's identifier.",
		Linked->ConnectedNodeIdentifier, B->GetNodeIdentifier())
	CHECK_EQUALS("ConnectedJunctionInstanceIdentifier must match the far junction that links back.",
		Linked->ConnectedJunctionInstanceIdentifier, 200)

	const FNCellLinkDetails* Free = FindByJunctionInstanceId(A->GetLinkDetails(), 101);
	if (Free == nullptr)
	{
		ADD_ERROR("Expected a link entry for the free junction with InstanceIdentifier 101.");
		return;
	}
	CHECK_FALSE_MESSAGE(TEXT("The unrelated free junction must remain unconnected."), Free->bConnected)
	CHECK_EQUALS("The free junction must leave ConnectedJunctionInstanceIdentifier at -1.",
		Free->ConnectedJunctionInstanceIdentifier, -1)
}

N_TEST_MEDIUM(FNAssemblyGraphCellNodeTests_GenerateLinkDetails_LinkToBoneLeavesJunctionUnset,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::GenerateLinkDetails::LinkToBoneLeavesJunctionUnset",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Linking to a non-cell node (a bone) records the connected node identifier and marks the junction connected —
	// a bone link counts as connected so the junction is not filled — but leaves ConnectedJunctionInstanceIdentifier
	// at -1 because bone nodes carry no junctions.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNGenerateLinkDetailsHarness;

	FNVirtualCellData CellA = MakeCellShell();
	AddJunction(CellA, 0, 100);

	FNAssemblyGraph Graph(MakeBone(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* A = AddNode(Graph, CellA);
	FNAssemblyGraphBoneNode* Bone = MakeBone();
	Graph.RegisterNode(Bone);
	A->LinkJunction(0, Bone);

	A->GenerateLinkDetails();

	const FNCellLinkDetails* Entry = FindByJunctionInstanceId(A->GetLinkDetails(), 100);
	if (Entry == nullptr)
	{
		ADD_ERROR("Expected a link entry for the junction with InstanceIdentifier 100.");
		return;
	}
	CHECK_MESSAGE(TEXT("A junction linked to a bone must report bConnected == true; bone links count as connected so the junction is not filled."),
		Entry->bConnected)
	CHECK_EQUALS("ConnectedNodeIdentifier must match the linked bone node's identifier.",
		Entry->ConnectedNodeIdentifier, Bone->GetNodeIdentifier())
	CHECK_EQUALS("A bone link must leave ConnectedJunctionInstanceIdentifier at -1.",
		Entry->ConnectedJunctionInstanceIdentifier, -1)
}

N_TEST_MEDIUM(FNAssemblyGraphCellNodeTests_GenerateLinkDetails_RegeneratingIsNoOp,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphCellNode::GenerateLinkDetails::RegeneratingIsNoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Once generated, a second call is a no-op and must not append duplicate entries.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNGenerateLinkDetailsHarness;

	FNVirtualCellData Cell = MakeCellShell();
	AddJunction(Cell, 0, 100);
	AddJunction(Cell, 1, 101);

	FNAssemblyGraph Graph(MakeBone(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	FNAssemblyGraphCellNode* Node = AddNode(Graph, Cell);

	Node->GenerateLinkDetails();
	Node->GenerateLinkDetails();

	CHECK_EQUALS("Regenerating must not append duplicate link entries.", Node->GetLinkDetails().Num(), 2)
}

#endif //WITH_TESTS
