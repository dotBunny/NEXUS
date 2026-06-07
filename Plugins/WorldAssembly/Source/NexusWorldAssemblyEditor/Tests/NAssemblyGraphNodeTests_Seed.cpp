// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Assembly/Graph/NAssemblyGraphNullNode.h"
#include "Cell/NCellAssemblyData.h"
#include "Types/NRawMeshUtils.h"
#include "Macros/NTestMacros.h"
#include "Math/NMersenneTwister.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNodeSeedHarness
{
	/** Distinctive seed used to prove a cell node stores exactly what the factory was handed. */
	constexpr uint64 SampleSeed = 0xABCDEF0123456789ull;

	/** Operation seed driving the deterministic per-node seed stream. */
	constexpr uint64 OperationSeed = 12345ull;

	/** A different operation seed, used to prove the stream actually depends on the seed. */
	constexpr uint64 AlternateOperationSeed = 99999ull;

	/** Number of cell nodes drawn in the determinism checks. */
	constexpr int32 NodeCount = 16;

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

N_TEST_CRITICAL(FNAssemblyGraphNodeTests_Seed_CellNodeRoundTripsSeed,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNode::Seed::CellNodeRoundTripsSeed",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies a cell node reports exactly the seed the factory was handed; this is the value the spawn
	// task later copies into FNCellAssemblyData, so any drift here silently de-syncs instance determinism.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNodeSeedHarness;

	FNVirtualCellData Cell = MakeCell();
	FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphNodeParams Params;
	Params.Seed = SampleSeed;
	FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
	Graph.RegisterNode(Node);

	CHECK_EQUALS("Cell node should report the seed it was constructed with.", Node->GetSeed(), SampleSeed)
}

N_TEST_HIGH(FNAssemblyGraphNodeTests_Seed_BoneAndNullDefaultToZero,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNode::Seed::BoneAndNullDefaultToZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Bone and null nodes are constructed with seed 0 ("no need for seed atm"). Pinning that lets a future
	// change that genuinely needs a bone/null seed surface as a test failure rather than a silent regression.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNodeSeedHarness;

	FNAssemblyGraphBoneNode* Bone = MakeBoneRoot();
	FNAssemblyGraph Graph(Bone, FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNAssemblyGraphNullNode* Null = FNAssemblyGraphNodeFactory::CreateNullNode(FVector::ZeroVector, FRotator::ZeroRotator);
	Graph.RegisterNode(Null);

	CHECK_EQUALS("Bone node should default to seed 0.", Bone->GetSeed(), static_cast<uint64>(0))
	CHECK_EQUALS("Null node should default to seed 0.", Null->GetSeed(), static_cast<uint64>(0))
}

N_TEST_HIGH(FNAssemblyGraphNodeTests_Seed_StreamIsDeterministic,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNode::Seed::StreamIsDeterministic",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mirrors how FNOrganGraphBuilderTask seeds each cell node (Random.UnsignedInteger64() per node). Two
	// builds from the same operation seed must produce the same ordered per-node seeds; a different
	// operation seed must produce a different stream. This is the contract the whole feature rests on.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNodeSeedHarness;

	FNVirtualCellData Cell = MakeCell();

	auto BuildSeeds = [&Cell](const uint64 InOperationSeed)
	{
		FNMersenneTwister Random(InOperationSeed);
		FNAssemblyGraph Graph(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

		TArray<uint64> Seeds;
		Seeds.Reserve(NodeCount);
		for (int32 i = 0; i < NodeCount; i++)
		{
			FNAssemblyGraphNodeParams Params;
			Params.Seed = Random.UnsignedInteger64();
			FNAssemblyGraphCellNode* Node = FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Cell, FVector(100.f));
			Graph.RegisterNode(Node);
			Seeds.Add(Node->GetSeed());
		}
		return Seeds;
	};

	const TArray<uint64> FirstRun = BuildSeeds(OperationSeed);
	const TArray<uint64> SecondRun = BuildSeeds(OperationSeed);
	const TArray<uint64> AlternateRun = BuildSeeds(AlternateOperationSeed);

	CHECK_MESSAGE(TEXT("The same operation seed must reproduce the identical per-node seed stream."),
		FirstRun == SecondRun)
	CHECK_FALSE_MESSAGE(TEXT("A different operation seed must produce a different per-node seed stream."),
		FirstRun == AlternateRun)

	// Each node draws a fresh value, so within a single build the assigned seeds should be unique.
	const TSet<uint64> Unique(FirstRun);
	CHECK_EQUALS("Each cell node should receive a distinct seed within a single build.", Unique.Num(), FirstRun.Num())
}

N_TEST_HIGH(FNAssemblyGraphNodeTests_Seed_DefaultCellAssemblyDataSeedIsZero,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraphNode::Seed::DefaultCellAssemblyDataSeedIsZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Pins the default-initialized Seed so a default FNCellAssemblyData can never carry a garbage seed onto an
	// instance. The real handoff (CellAssemblyData.Seed = CellNode->GetSeed()) lives in
	// FNSpawnCellProxiesTask::DoTask and runs inside a spawning world, so it is not exercised here; the node side
	// of that copy — that GetSeed() returns exactly what the factory was handed — is covered by
	// CellNodeRoundTripsSeed above.
	FNCellAssemblyData DefaultData;
	CHECK_EQUALS("A default-constructed FNCellAssemblyData should report seed 0.", DefaultData.Seed, static_cast<uint64>(0))
}

#endif //WITH_TESTS
