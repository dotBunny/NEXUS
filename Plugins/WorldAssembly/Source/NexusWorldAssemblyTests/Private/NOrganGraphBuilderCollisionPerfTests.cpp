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
#include "Macros/NTestMacros.h"
#include "Types/NRawMeshUtils.h"

/**
 * Baseline for the per-candidate collision scans in FNOrganGraphBuilderTask.
 *
 * Every candidate cell placement is validated by scanning the entire world-collision set (DoesWorldCollide), the
 * entire set of cells placed by earlier passes (DoesExistingNodeWorldCollide), and every node in the organ graph
 * (CheckNodeBounds). All three are linear in their collection, every rejected candidate pays all of them, and a
 * failed organ retries the whole build — so they compound.
 *
 * These measure the miss path: candidates whose AABB overlaps nothing. That is deliberate. It is what the builder
 * pays on the overwhelming majority of candidates, it is the pure per-element scan overhead with no deeper geometry
 * work mixed in, and it is exactly the cost an object-level broadphase removes.
 *
 * DoesExistingNodeWorldCollide is not measured directly: its loop is structurally identical to DoesWorldCollide's
 * over a different array, and it reads a count snapshotted inside DoTask that a direct call cannot populate without
 * changing production behaviour. DoesWorldCollide stands in for both.
 */
namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness
{
	// World-collision mesh counts standing in for a sparse, a busy, and a densely dressed authored level. One mesh
	// is emitted per collision primitive, so the dense figure is not unusual inside a large organ volume.
	constexpr int32 SmallWorldMeshCount = 100;
	constexpr int32 MediumWorldMeshCount = 1000;
	constexpr int32 LargeWorldMeshCount = 5000;

	// Placed-cell counts standing in for a small and a large organ mid-build.
	constexpr int32 SmallGraphCellCount = 100;
	constexpr int32 LargeGraphCellCount = 1000;

	// Iteration counts fall as the collection grows so every scenario does comparable total work and no single test
	// dominates the suite's runtime. Sized so each timed loop lands in the tens of milliseconds — a sub-millisecond
	// measurement carries too much timer jitter to gate on.
	constexpr int32 SmallIterations = 10000;
	constexpr int32 MediumIterations = 5000;
	constexpr int32 LargeIterations = 2000;

	// MaxDuration values are in milliseconds and bound the total cost of the inner loop (NOT per call). Sized to
	// roughly 4x the observed baseline — enough headroom to absorb CI / contended-core jitter without false-failing,
	// tight enough to catch a genuine regression. These are gates, not the trend record; fine-grained drift is
	// tracked by the telemetry the perf run emits per scope.
	// The three DoesWorldCollide gates are set against post-broadphase timings, generously enough to absorb jitter
	// on a sub-millisecond measurement but far below the brute-force reference below — so a regression that
	// reintroduced the linear scan fails loudly instead of sliding under a gate sized for the old behaviour.
	constexpr float SmallWorldMaxDuration = 3.0f;
	constexpr float MediumWorldMaxDuration = 3.0f;
	constexpr float LargeWorldMaxDuration = 3.0f;
	// Set against post-index timings, well below the ~2us-per-call linear scan they replaced, so a regression that
	// reintroduced the walk fails rather than sliding under a gate sized for the old behaviour.
	constexpr float SmallGraphMaxDuration = 3.0f;
	constexpr float LargeGraphMaxDuration = 3.0f;

	/** Loose by design: this scenario exists to record what the scan costs, not to police it. */
	constexpr float LargeWorldBruteForceMaxDuration = 600.0f;

	/** Per-candidate construction cost; the denominator the scan timings must be read against. */
	constexpr float CandidateMaxDuration = 70.0f;

	/** Half-extent of every box hull used here, for both world collision and cell hulls. */
	constexpr double HullHalfExtent = 50.0;

	/** Spacing between scattered meshes; comfortably wider than HullHalfExtent so nothing overlaps anything else. */
	constexpr double GridSpacing = 300.0;

	/**
	 * @return A deterministic position on a cube lattice **centred on the origin**.
	 * @note The centring matters. The measured candidate sits at CandidateOffset, just inside the lattice, so a
	 *       spatial index has to actually descend the tree to answer. An earlier revision scattered the lattice
	 *       away from the candidate, which let a broadphase reject the entire set on the root node in a single
	 *       test — a real answer, but a best case that flattered the tree and told us nothing about the work it
	 *       does on a candidate placed inside the level. A linear scan is indifferent to this, which is exactly
	 *       why the baseline did not expose it.
	 */
	static FVector GridPosition(const int32 Index, const int32 Count)
	{
		const int32 Side = FMath::Max(1, FMath::CeilToInt(FMath::Pow(static_cast<double>(Count), 1.0 / 3.0)));
		// Integer recentring, deliberately. A fractional centre shifts the lattice by half a step when Side is even,
		// which moves a mesh onto the candidate and turns the scenario into a collision-with-deep-test instead of the
		// miss it is meant to measure. Anchoring on integer multiples of GridSpacing keeps CandidatePosition's
		// half-step offset exactly mid-gap for every Side.
		const int32 Centre = Side / 2;
		const int32 X = Index % Side;
		const int32 Y = (Index / Side) % Side;
		const int32 Z = (Index / (Side * Side)) % Side;
		return FVector((X - Centre) * GridSpacing, (Y - Centre) * GridSpacing, (Z - Centre) * GridSpacing);
	}

	/**
	 * Candidate position: half a lattice step off the grid, so it sits *inside* the scattered volume but in the
	 * gap between neighbours. That keeps the measurement on the miss path — what the builder pays for the vast
	 * majority of candidates — while still forcing a spatial index to traverse rather than reject at the root.
	 */
	static FVector CandidatePosition()
	{
		return FVector(GridSpacing * 0.5);
	}

	/**
	 * Junctions per cell. Six — one per face of a box cell — rather than one, because the per-candidate cost this
	 * file measures includes populating the junction map, and a single-junction cell would understate what an
	 * authored cell actually costs to construct.
	 */
	constexpr int32 JunctionsPerCell = 6;

	/** @return A cell template with deterministic box bounds and hull, and one junction per face. */
	static FNVirtualCellData MakeCell()
	{
		FNVirtualCellData Cell;
		Cell.CellDetails.Bounds = FBox(FVector(-HullHalfExtent), FVector(HullHalfExtent));
		Cell.CellDetails.Hull = FNRawMeshUtils::MakeBoxHull(Cell.CellDetails.Bounds);

		static const FVector FaceNormals[6] = {
			FVector(1, 0, 0), FVector(-1, 0, 0), FVector(0, 1, 0),
			FVector(0, -1, 0), FVector(0, 0, 1), FVector(0, 0, -1)
		};
		for (int32 i = 0; i < JunctionsPerCell; ++i)
		{
			FNCellJunctionDetails Junction;
			Junction.WorldLocation = FaceNormals[i] * HullHalfExtent;
			Junction.WorldRotation = FaceNormals[i].Rotation();
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

	/**
	 * @return A world context carrying MeshCount scattered, baked, cache-warmed box hulls.
	 * @note Warmed exactly as FNProcessVirtualWorldTask warms them, so the measurement is the query cost rather
	 *       than a one-off lazy cache build on the first iteration. Warmed in place because FNRawMesh's copy
	 *       constructor deliberately drops the face-plane cache.
	 */
	static TSharedPtr<FNVirtualWorldContext> MakeWorldContext(const int32 MeshCount)
	{
		static FNAssemblyOperationSettings Settings;
		const TArray<FBoxSphereBounds> NoBounds;
		TSharedPtr<FNVirtualWorldContext> WorldContext =
			MakeShared<FNVirtualWorldContext>(static_cast<UWorld*>(nullptr), NoBounds, Settings);

		WorldContext->WorldCollisionMeshes.Reserve(MeshCount);
		TArray<FBox> MeshBounds;
		MeshBounds.Reserve(MeshCount);
		for (int32 i = 0; i < MeshCount; ++i)
		{
			const FVector Center = GridPosition(i, MeshCount);
			const int32 Index = WorldContext->WorldCollisionMeshes.Add(
				FNRawMeshUtils::MakeBoxHull(FBox(Center - FVector(HullHalfExtent), Center + FVector(HullHalfExtent))));
			WorldContext->WorldCollisionMeshes[Index].EnsureValidated();
			WorldContext->WorldCollisionMeshes[Index].EnsureCachedFacePlanes();
			MeshBounds.Add(WorldContext->WorldCollisionMeshes[Index].Bounds);
		}

		// Build the broadphase the same way FNProcessVirtualWorldTask does. Without this the tree is empty, every
		// query returns nothing, and DoesWorldCollide answers "clear" without testing a single mesh — a number that
		// looks like a spectacular speedup and measures absolutely nothing.
		WorldContext->WorldCollisionBVH = FNBoundsBVH(MeshBounds);
		return WorldContext;
	}

	/** @return An organ context whose graph already holds CellCount scattered, registered cell nodes. */
	static TSharedPtr<FNVirtualOrganContext> MakeOrganContext(const int32 CellCount)
	{
		TSharedPtr<FNVirtualOrganContext> Context =
			MakeShared<FNVirtualOrganContext>(4242ull, FString(TEXT("OrganGraphBuilderCollisionPerf")));
		Context->bUnbound = true;

		// Exactly one template, added before anything takes its address, so the pointer handed to each node stays valid.
		Context->CellInputData.Add(MakeCell());

		Context->CellGraph = MakeUnique<FNAssemblyGraph>(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
		for (int32 i = 0; i < CellCount; ++i)
		{
			FNAssemblyGraphNodeParams Params;
			Params.WorldPosition = GridPosition(i, CellCount);
			Context->CellGraph->RegisterNode(
				FNAssemblyGraphNodeFactory::CreateCellNode(Params, &Context->CellInputData[0], FVector(HullHalfExtent)));
		}
		return Context;
	}

	/** Construct a builder task over the supplied contexts, mirroring how the task graph builds one per organ. */
	static TUniquePtr<FNOrganGraphBuilderTask> MakeTask(const TSharedPtr<FNVirtualOrganContext>& OrganContext,
		const TSharedPtr<FNVirtualWorldContext>& WorldContext)
	{
		static FNAssemblyOperationSettings Settings;
		const TSharedPtr<FNPassContext> PassContext = MakeShared<FNPassContext>();
		int32 Ticket = 0;
		const TSharedPtr<FNAssemblyTaskGraphContext> TaskGraphContext =
			MakeShared<FNAssemblyTaskGraphContext>(static_cast<UWorld*>(nullptr), Ticket, Settings);
		const TSharedPtr<FNAssemblyTaskAnalytics> Analytics =
			MakeShared<FNAssemblyTaskAnalytics, ESPMode::ThreadSafe>(FText::FromString(TEXT("CollisionPerf")));
		return MakeUnique<FNOrganGraphBuilderTask>(OrganContext, PassContext, WorldContext, TaskGraphContext, Analytics);
	}

	/**
	 * Everything one measured scenario needs, kept alive for the duration of the timed loop.
	 * The candidate is deleted on teardown, matching how the builder disposes of a rejected placement.
	 */
	struct FScenario
	{
		TSharedPtr<FNVirtualOrganContext> OrganContext;
		TSharedPtr<FNVirtualWorldContext> WorldContext;
		TUniquePtr<FNOrganGraphBuilderTask> Task;
		FNAssemblyGraphCellNode* Candidate = nullptr;

		FScenario() = default;
		FScenario(const FScenario&) = delete;
		FScenario& operator=(const FScenario&) = delete;

		// Declaring a destructor suppresses the implicit move constructor, which the factory's return needs.
		FScenario(FScenario&& Other) noexcept
			: OrganContext(MoveTemp(Other.OrganContext))
			, WorldContext(MoveTemp(Other.WorldContext))
			, Task(MoveTemp(Other.Task))
			, Candidate(Other.Candidate)
		{
			Other.Candidate = nullptr;
		}

		~FScenario() { delete Candidate; }
	};

	/** @return A scenario with WorldMeshCount world-collision hulls and GraphCellCount placed cells, candidate at the origin. */
	static FScenario MakeScenario(const int32 WorldMeshCount, const int32 GraphCellCount)
	{
		FScenario Scenario;
		Scenario.OrganContext = MakeOrganContext(GraphCellCount);
		Scenario.WorldContext = MakeWorldContext(WorldMeshCount);
		Scenario.Task = MakeTask(Scenario.OrganContext, Scenario.WorldContext);

		FNAssemblyGraphNodeParams Params;
		Params.WorldPosition = CandidatePosition();
		Scenario.Candidate = FNAssemblyGraphNodeFactory::CreateCellNode(
			Params, &Scenario.OrganContext->CellInputData[0], FVector(HullHalfExtent));

		// A scenario that silently failed to populate would report a fast, meaningless number and read as a pass.
		// Assert the collections the scans walk are actually the size this measurement claims to be about.
		checkf(Scenario.WorldContext->WorldCollisionMeshes.Num() == WorldMeshCount,
			TEXT("World-collision scenario built %d meshes, expected %d."),
			Scenario.WorldContext->WorldCollisionMeshes.Num(), WorldMeshCount);
		checkf(Scenario.WorldContext->WorldCollisionBVH.Num() == WorldMeshCount,
			TEXT("World-collision broadphase indexed %d meshes, expected %d. An unbuilt tree makes every query "
				 "return nothing, which reads as a huge speedup while measuring no work at all."),
			Scenario.WorldContext->WorldCollisionBVH.Num(), WorldMeshCount);
		checkf(Scenario.Candidate == nullptr || WorldMeshCount == 0
			|| Scenario.WorldContext->WorldCollisionBVH.GetBounds().IsInsideOrOn(Scenario.Candidate->GetHullBounds().GetCenter()),
			TEXT("The candidate must sit inside the scattered volume, or the broadphase rejects everything at the root."));

		// These scenarios document themselves as measuring the miss path — the one the builder pays on the vast
		// majority of candidates. If the candidate landed on a mesh instead, the timing would be dominated by the
		// deep hull test and would not be comparable across sizes.
		checkf(WorldMeshCount == 0 || !Scenario.Task->DoesWorldCollide(Scenario.Candidate),
			TEXT("The candidate collides, so this scenario measures the deep hull test rather than the scan."));
		checkf(Scenario.OrganContext->CellGraph->GetCellNodeCount() == GraphCellCount,
			TEXT("Graph scenario built %d cell nodes, expected %d."),
			Scenario.OrganContext->CellGraph->GetCellNodeCount(), GraphCellCount);
		checkf(Scenario.Candidate != nullptr, TEXT("Scenario failed to build a candidate cell node."));

		return Scenario;
	}
}

class FNOrganGraphBuilderCollisionPerfTests
{
public:
	static void DoesWorldCollide_Small()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(SmallWorldMeshCount, 0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_DoesWorldCollide_Small, SmallWorldMaxDuration)
			for (int32 i = 0; i < SmallIterations; ++i)
			{
				Scenario.Task->DoesWorldCollide(Scenario.Candidate);
			}
			NTestTimer.ManualStop();
		}
	}

	static void DoesWorldCollide_Medium()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(MediumWorldMeshCount, 0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_DoesWorldCollide_Medium, MediumWorldMaxDuration)
			for (int32 i = 0; i < MediumIterations; ++i)
			{
				Scenario.Task->DoesWorldCollide(Scenario.Candidate);
			}
			NTestTimer.ManualStop();
		}
	}

	static void DoesWorldCollide_Large()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(LargeWorldMeshCount, 0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_DoesWorldCollide_Large, LargeWorldMaxDuration)
			for (int32 i = 0; i < LargeIterations; ++i)
			{
				Scenario.Task->DoesWorldCollide(Scenario.Candidate);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * Construct and immediately discard a candidate cell node — what TryAttachCellToJunction does for every
	 * candidate before any collision test runs, and again for every candidate a test rejects.
	 *
	 * This is the denominator the collision scans have to be read against. Optimising a scan that turns out to be
	 * a small share of the per-candidate cost cannot move total generation time, however good the scan's own
	 * speedup looks in isolation.
	 */
	static void CreateAndDiscardCandidate()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(0, 0);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_CreateAndDiscardCandidate, CandidateMaxDuration)
			for (int32 i = 0; i < SmallIterations; ++i)
			{
				FNAssemblyGraphNodeParams Params;
				Params.WorldPosition = CandidatePosition();
				const FNAssemblyGraphCellNode* Candidate = FNAssemblyGraphNodeFactory::CreateCellNode(
					Params, &Scenario.OrganContext->CellInputData[0], FVector(HullHalfExtent));
				delete Candidate;
			}
			NTestTimer.ManualStop();
		}
	}

	static void CheckNodeBounds_Small()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(0, SmallGraphCellCount);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_CheckNodeBounds_Small, SmallGraphMaxDuration)
			for (int32 i = 0; i < SmallIterations; ++i)
			{
				Scenario.Task->CheckNodeBounds(Scenario.Candidate);
			}
			NTestTimer.ManualStop();
		}
	}

	/**
	 * The linear scan DoesWorldCollide used to be, over the same 5000 meshes and the same candidate. Kept as the
	 * permanent reference the broadphase is measured against: the ratio between this and DoesWorldCollide_Large
	 * is the win, stated in one place rather than inferred from a changelist history.
	 */
	static void DoesWorldCollide_Large_BruteForce()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(LargeWorldMeshCount, 0);
		const TArray<FNRawMesh>& Meshes = Scenario.WorldContext->WorldCollisionMeshes;
		const float MaxPenetration = Scenario.OrganContext->WorldHullPenetration;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_DoesWorldCollide_Large_BruteForce, LargeWorldBruteForceMaxDuration)
			for (int32 i = 0; i < LargeIterations; ++i)
			{
				for (int32 m = 0; m < Meshes.Num(); ++m)
				{
					const float Depth = Scenario.Candidate->GetHullIntersectDepth(Meshes[m], MaxPenetration);
					if (Depth == 0.0f)
					{
						if (Scenario.Candidate->CheckHullIntersects(Meshes[m])) break;
						continue;
					}
					if (Depth >= MaxPenetration) break;
				}
			}
			NTestTimer.ManualStop();
		}
	}

	static void CheckNodeBounds_Large()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderCollisionHarness;
		const FScenario Scenario = MakeScenario(0, LargeGraphCellCount);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNOrganGraphBuilderTask_CheckNodeBounds_Large, LargeGraphMaxDuration)
			for (int32 i = 0; i < LargeIterations; ++i)
			{
				Scenario.Task->CheckNodeBounds(Scenario.Candidate);
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_DoesWorldCollide_Small,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesWorldCollide_Small",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::DoesWorldCollide_Small));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_DoesWorldCollide_Medium,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesWorldCollide_Medium",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::DoesWorldCollide_Medium));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_DoesWorldCollide_Large,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesWorldCollide_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::DoesWorldCollide_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_DoesWorldCollide_Large_BruteForce,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::DoesWorldCollide_Large_BruteForce",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::DoesWorldCollide_Large_BruteForce));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_CreateAndDiscardCandidate,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::CreateAndDiscardCandidate",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::CreateAndDiscardCandidate));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_CheckNodeBounds_Small,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::CheckNodeBounds_Small",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::CheckNodeBounds_Small));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNOrganGraphBuilderCollisionPerfTests_CheckNodeBounds_Large,
	"NEXUS::PerfTests::NWorldAssembly::FNOrganGraphBuilderTask::CheckNodeBounds_Large",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNOrganGraphBuilderCollisionPerfTests::CheckNodeBounds_Large));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
