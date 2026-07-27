// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/NAssemblyOperationSettings.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/Contexts/NPassContext.h"
#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphBoneNode.h"
#include "Assembly/Graph/NAssemblyGraphCellNode.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Assembly/Tasks/NOrganGraphBuilderTask.h"
#include "Math/NBoundsBVH.h"
#include "Math/NMersenneTwister.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness
{
	constexpr double HullHalfExtent = 50.0;

	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-HullHalfExtent), FVector(HullHalfExtent));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);
		return Cell;
	}

	/**
	 * The original linear scan, reimplemented here verbatim. The broadphase in FNOrganGraphBuilderTask must agree
	 * with this on every candidate — that agreement is the whole correctness claim of the change.
	 */
	static bool BruteForceWorldCollides(const FNAssemblyGraphCellNode* CellNode,
		const TArray<FNRawMesh>& WorldCollisionMeshes, const float WorldHullPenetration)
	{
		for (int32 i = 0; i < WorldCollisionMeshes.Num(); i++)
		{
			const float PenetrationDepth = CellNode->GetHullIntersectDepth(WorldCollisionMeshes[i], WorldHullPenetration);
			if (PenetrationDepth == 0.0f)
			{
				if (CellNode->CheckHullIntersects(WorldCollisionMeshes[i]))
				{
					return true;
				}
				continue;
			}
			if (PenetrationDepth >= WorldHullPenetration)
			{
				return true;
			}
		}
		return false;
	}

	/** Populates World with MeshCount baked, warmed box hulls scattered over Extent, and builds its broadphase. */
	static void PopulateWorld(const TSharedPtr<FNVirtualWorldContext>& World, const int32 MeshCount,
		const double Extent, const uint64 Seed)
	{
		FNMersenneTwister Random(Seed);
		World->WorldCollisionMeshes.Reserve(MeshCount);
		TArray<FBox> MeshBounds;
		MeshBounds.Reserve(MeshCount);

		for (int32 i = 0; i < MeshCount; ++i)
		{
			const FVector Center(
				Random.DoubleRange(-Extent, Extent),
				Random.DoubleRange(-Extent, Extent),
				Random.DoubleRange(-Extent, Extent));
			const double Half = Random.DoubleRange(30.0, 90.0);
			const int32 Index = World->WorldCollisionMeshes.Add(
				FNRawMeshUtils::MakeBoxHull(FBox(Center - FVector(Half), Center + FVector(Half))));
			World->WorldCollisionMeshes[Index].EnsureValidated();
			World->WorldCollisionMeshes[Index].EnsureCachedFacePlanes();
			MeshBounds.Add(World->WorldCollisionMeshes[Index].Bounds);
		}

		World->WorldCollisionBVH = FNBoundsBVH(MeshBounds);
	}

	static TSharedPtr<FNVirtualWorldContext> MakeWorldContext()
	{
		static FNAssemblyOperationSettings Settings;
		const TArray<FBoxSphereBounds> NoBounds;
		return MakeShared<FNVirtualWorldContext>(static_cast<UWorld*>(nullptr), NoBounds, Settings);
	}

	static TUniquePtr<FNOrganGraphBuilderTask> MakeTask(const TSharedPtr<FNVirtualOrganContext>& OrganContext,
		const TSharedPtr<FNVirtualWorldContext>& WorldContext)
	{
		static FNAssemblyOperationSettings Settings;
		const TSharedPtr<FNPassContext> PassContext = MakeShared<FNPassContext>();
		int32 Ticket = 0;
		const TSharedPtr<FNAssemblyTaskGraphContext> TaskGraphContext =
			MakeShared<FNAssemblyTaskGraphContext>(static_cast<UWorld*>(nullptr), Ticket, Settings);
		const TSharedPtr<FNAssemblyTaskAnalytics> Analytics =
			MakeShared<FNAssemblyTaskAnalytics, ESPMode::ThreadSafe>(FText::FromString(TEXT("CollisionEquivalence")));
		return MakeUnique<FNOrganGraphBuilderTask>(OrganContext, PassContext, WorldContext, TaskGraphContext, Analytics);
	}
}

N_TEST_CRITICAL(FNOrganGraphBuilderCollisionTests_DoesWorldCollide_MatchesBruteForce,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesWorldCollide::MatchesBruteForce",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The correctness claim of the broadphase: for every candidate placement it must reach the same verdict as the
	// linear scan it replaced. Densities are chosen so the sample contains both collisions and clean placements —
	// a run that was all one or all the other would prove nothing.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;

	const TSharedPtr<FNVirtualOrganContext> OrganContext =
		MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("CollisionEquivalence")));
	OrganContext->bUnbound = true;
	OrganContext->CellInputData.Add(MakeCell());

	const TSharedPtr<FNVirtualWorldContext> WorldContext = MakeWorldContext();
	PopulateWorld(WorldContext, 300, 600.0, 20260727ull);

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(OrganContext, WorldContext);
	const float WorldHullPenetration = OrganContext->WorldHullPenetration;

	FNMersenneTwister Random(1234ull);
	int32 Collisions = 0;
	int32 Clears = 0;

	for (int32 Attempt = 0; Attempt < 400; ++Attempt)
	{
		FNAssemblyGraphNodeParams Params;
		Params.WorldPosition = FVector(
			Random.DoubleRange(-700.0, 700.0),
			Random.DoubleRange(-700.0, 700.0),
			Random.DoubleRange(-700.0, 700.0));
		FNAssemblyGraphCellNode* Candidate =
			FNAssemblyGraphNodeFactory::CreateCellNode(Params, &OrganContext->CellInputData[0], FVector(HullHalfExtent));

		const bool bBroadphase = Task->DoesWorldCollide(Candidate);
		const bool bBruteForce = BruteForceWorldCollides(Candidate, WorldContext->WorldCollisionMeshes, WorldHullPenetration);
		delete Candidate;

		if (bBroadphase != bBruteForce)
		{
			ADD_ERROR(FString::Printf(
				TEXT("Placement %d disagreed: broadphase said %s, the linear scan said %s."),
				Attempt, bBroadphase ? TEXT("collide") : TEXT("clear"), bBruteForce ? TEXT("collide") : TEXT("clear")));
			break;
		}

		bBruteForce ? ++Collisions : ++Clears;
	}

	CHECK_MESSAGE(TEXT("The sample must contain colliding placements, or agreement proves nothing."), Collisions > 0)
	CHECK_MESSAGE(TEXT("The sample must contain clear placements, or agreement proves nothing."), Clears > 0)
}

N_TEST_CRITICAL(FNOrganGraphBuilderCollisionTests_DoesExistingNodeWorldCollide_MatchesBruteForce,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesExistingNodeWorldCollide::MatchesBruteForce",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Same claim as the world-collision test, for the placed-cell scan. Structurally identical code, but verified
	// independently rather than assumed to follow — the two differ in which array they read and which snapshot
	// bounds them, and either could be wired up wrongly without the other noticing.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;

	const TSharedPtr<FNVirtualOrganContext> OrganContext =
		MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("NodeCollisionEquivalence")));
	OrganContext->bUnbound = true;
	OrganContext->CellInputData.Add(MakeCell());

	// Placed-cell hulls live on the world context's node array; the broadphase and its extent live on the organ
	// context, mirroring what FNOrganGraphBuilderTask::DoTask sets up at task start.
	const TSharedPtr<FNVirtualWorldContext> WorldContext = MakeWorldContext();
	FNMersenneTwister Layout(777ull);
	TArray<FBox> NodeBounds;
	for (int32 i = 0; i < 250; ++i)
	{
		const FVector Center(
			Layout.DoubleRange(-600.0, 600.0),
			Layout.DoubleRange(-600.0, 600.0),
			Layout.DoubleRange(-600.0, 600.0));
		const double Half = Layout.DoubleRange(30.0, 90.0);
		const int32 Index = WorldContext->NodeCollisionMeshes.Add(
			FNRawMeshUtils::MakeBoxHull(FBox(Center - FVector(Half), Center + FVector(Half))));
		WorldContext->NodeCollisionMeshes[Index].EnsureValidated();
		WorldContext->NodeCollisionMeshes[Index].EnsureCachedFacePlanes();
		NodeBounds.Add(WorldContext->NodeCollisionMeshes[Index].Bounds);
	}
	OrganContext->NodeCollisionSnapshotCount = WorldContext->NodeCollisionMeshes.Num();
	OrganContext->NodeCollisionBVH = FNBoundsBVH(NodeBounds);

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(OrganContext, WorldContext);
	const float CellHullPenetration = OrganContext->CellHullPenetration;

	FNMersenneTwister Random(555ull);
	int32 Collisions = 0;
	int32 Clears = 0;

	for (int32 Attempt = 0; Attempt < 400; ++Attempt)
	{
		FNAssemblyGraphNodeParams Params;
		Params.WorldPosition = FVector(
			Random.DoubleRange(-700.0, 700.0),
			Random.DoubleRange(-700.0, 700.0),
			Random.DoubleRange(-700.0, 700.0));
		FNAssemblyGraphCellNode* Candidate =
			FNAssemblyGraphNodeFactory::CreateCellNode(Params, &OrganContext->CellInputData[0], FVector(HullHalfExtent));

		const bool bBroadphase = Task->DoesExistingNodeWorldCollide(Candidate);
		const bool bBruteForce = BruteForceWorldCollides(Candidate, WorldContext->NodeCollisionMeshes, CellHullPenetration);
		delete Candidate;

		if (bBroadphase != bBruteForce)
		{
			ADD_ERROR(FString::Printf(
				TEXT("Placement %d disagreed: broadphase said %s, the linear scan said %s."),
				Attempt, bBroadphase ? TEXT("collide") : TEXT("clear"), bBruteForce ? TEXT("collide") : TEXT("clear")));
			break;
		}

		bBruteForce ? ++Collisions : ++Clears;
	}

	CHECK_MESSAGE(TEXT("The sample must contain colliding placements, or agreement proves nothing."), Collisions > 0)
	CHECK_MESSAGE(TEXT("The sample must contain clear placements, or agreement proves nothing."), Clears > 0)
}

N_TEST_HIGH(FNOrganGraphBuilderCollisionTests_DoesExistingNodeWorldCollide_RespectsSnapshot,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesExistingNodeWorldCollide::RespectsSnapshot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Hulls appended after this build snapshotted the array belong to a later pass and must stay invisible to it.
	// The broadphase indexes only the snapshot, so a later append that the tree does not know about must not leak
	// into the scan — which is exactly what a tree built over the whole array would do.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;

	const TSharedPtr<FNVirtualOrganContext> OrganContext =
		MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("NodeCollisionSnapshot")));
	OrganContext->bUnbound = true;
	OrganContext->CellInputData.Add(MakeCell());

	const TSharedPtr<FNVirtualWorldContext> WorldContext = MakeWorldContext();

	// Snapshot taken while the array is empty: nothing is visible to this build.
	OrganContext->NodeCollisionSnapshotCount = 0;
	OrganContext->NodeCollisionBVH = FNBoundsBVH(TArray<FBox>{});

	// A later pass then appends a hull sitting right on top of the candidate.
	WorldContext->NodeCollisionMeshes.Add(FNRawMeshUtils::MakeBoxHull(FBox(FVector(-100.0), FVector(100.0))));

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(OrganContext, WorldContext);

	FNAssemblyGraphNodeParams Params;
	Params.WorldPosition = FVector::ZeroVector;
	FNAssemblyGraphCellNode* Candidate =
		FNAssemblyGraphNodeFactory::CreateCellNode(Params, &OrganContext->CellInputData[0], FVector(HullHalfExtent));

	const bool bCollides = Task->DoesExistingNodeWorldCollide(Candidate);
	delete Candidate;

	CHECK_FALSE_MESSAGE(TEXT("A hull appended after the snapshot must not be visible to this build."), bCollides)
}

N_TEST_CRITICAL(FNOrganGraphBuilderCollisionTests_QueryCellNodesByBounds_MatchesLinearScan,
	"NEXUS::UnitTests::NWorldAssembly::FNAssemblyGraph::QueryCellNodesByBounds::MatchesLinearScan",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The graph's spatial index must report exactly the set a walk of GetNodes() would. Registrations are
	// interleaved with queries so the index is exercised in every state it can be in: freshly rebuilt, carrying a
	// partial unindexed tail, and invalidated by a removal.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;

	const TSharedPtr<FNVirtualOrganContext> OrganContext =
		MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("CellNodeIndex")));
	OrganContext->bUnbound = true;
	OrganContext->CellInputData.Add(MakeCell());

	// A real bone root, not nullptr: the graph constructor adds whatever it is given to Nodes unconditionally, so a
	// null root leaves a null entry that every GetNodes() consumer would dereference. Production always passes one.
	static FNVirtualBoneData BoneData;
	BoneData.SocketSize = FIntVector2(2, 4);
	FNAssemblyGraph Graph(FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator),
		FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);

	FNMersenneTwister Random(31337ull);
	TArray<FNAssemblyGraphCellNode*> Registered;

	// 300 registrations comfortably crosses the rebuild threshold several times.
	for (int32 Step = 0; Step < 300; ++Step)
	{
		FNAssemblyGraphNodeParams Params;
		Params.WorldPosition = FVector(
			Random.DoubleRange(-800.0, 800.0),
			Random.DoubleRange(-800.0, 800.0),
			Random.DoubleRange(-800.0, 800.0));
		FNAssemblyGraphCellNode* Node =
			FNAssemblyGraphNodeFactory::CreateCellNode(Params, &OrganContext->CellInputData[0], FVector(HullHalfExtent));
		Graph.RegisterNode(Node);
		Registered.Add(Node);

		// Periodically remove one, which invalidates the index and forces the rebuild path.
		if (Step > 0 && Step % 50 == 0)
		{
			FNAssemblyGraphCellNode* Doomed = Registered[Random.IntegerRange(0, Registered.Num() - 1)];
			Graph.UnregisterNode(Doomed);
			Registered.Remove(Doomed);
			delete Doomed;
		}

		// Query with a box that overlaps a meaningful share of the registered nodes.
		const FVector QueryCentre(
			Random.DoubleRange(-800.0, 800.0),
			Random.DoubleRange(-800.0, 800.0),
			Random.DoubleRange(-800.0, 800.0));
		const FBox QueryBox(QueryCentre - FVector(200.0), QueryCentre + FVector(200.0));

		TArray<FNAssemblyGraphCellNode*> Indexed;
		Graph.QueryCellNodesByBounds(QueryBox, Indexed);

		TArray<FNAssemblyGraphCellNode*> Expected;
		for (FNAssemblyGraphNode* GraphNode : Graph.GetNodes())
		{
			if (GraphNode->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;
			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(GraphNode);
			if (CellNode->CheckBoundsIntersects(QueryBox))
			{
				Expected.Add(CellNode);
			}
		}

		// Compare as sets: the index makes no ordering promise, only that the membership matches. Compared by
		// membership rather than by sorting, because TArray<T*>::Sort dereferences its elements.
		bool bMatches = Indexed.Num() == Expected.Num();
		if (bMatches)
		{
			for (const FNAssemblyGraphCellNode* ExpectedNode : Expected)
			{
				if (!Indexed.Contains(ExpectedNode))
				{
					bMatches = false;
					break;
				}
			}
		}

		if (!bMatches)
		{
			ADD_ERROR(FString::Printf(TEXT("Step %d disagreed with the linear scan (indexed %d, expected %d)."),
				Step, Indexed.Num(), Expected.Num()));
			break;
		}
	}
}

N_TEST_HIGH(FNOrganGraphBuilderCollisionTests_DoesWorldCollide_UnboundedMeshStillTested,
	"NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesWorldCollide::UnboundedMeshStillTested",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A mesh with no valid bounds cannot be indexed by the broadphase, so it must be carried in the unbounded list
	// and tested unconditionally. Without that, dropping it from the tree would silently drop it from the scan.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;

	const TSharedPtr<FNVirtualOrganContext> OrganContext =
		MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("CollisionEquivalence")));
	OrganContext->bUnbound = true;
	OrganContext->CellInputData.Add(MakeCell());

	const TSharedPtr<FNVirtualWorldContext> WorldContext = MakeWorldContext();

	// One mesh straddling the origin, deliberately stripped of its bounds so it can only be reached via the
	// unbounded path. A candidate at the origin overlaps it.
	FNRawMesh Overlapping = FNRawMeshUtils::MakeBoxHull(FBox(FVector(-100.0), FVector(100.0)));
	Overlapping.Bounds = FBox(ForceInit);
	WorldContext->WorldCollisionMeshes.Add(MoveTemp(Overlapping));
	WorldContext->UnboundedWorldCollisionIndices.Add(0);
	WorldContext->WorldCollisionBVH = FNBoundsBVH(TArray<FBox>{ FBox(ForceInit) });

	CHECK_MESSAGE(TEXT("A mesh with stripped bounds must not be indexed by the broadphase."),
		WorldContext->WorldCollisionBVH.IsEmpty())

	const TUniquePtr<FNOrganGraphBuilderTask> Task = MakeTask(OrganContext, WorldContext);

	FNAssemblyGraphNodeParams Params;
	Params.WorldPosition = FVector::ZeroVector;
	FNAssemblyGraphCellNode* Candidate =
		FNAssemblyGraphNodeFactory::CreateCellNode(Params, &OrganContext->CellInputData[0], FVector(HullHalfExtent));

	const bool bCollides = Task->DoesWorldCollide(Candidate);
	delete Candidate;

	CHECK_MESSAGE(TEXT("An unbounded mesh the candidate overlaps must still register as a collision."), bCollides)
}

#endif //WITH_TESTS
