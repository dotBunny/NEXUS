// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/NJunctionConnectorSolver.h"
#include "Macros/NTestMacros.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolverHarness
{
	/** Candidate pairs routed per run; a dense layout can easily present this many before the greedy walk settles. */
	constexpr int32 RouteCount = 250;

	/** Distance between the two sockets of each routed pair. */
	constexpr double PairDistance = 600.0;

	/** Routing is cheap — curve maths and four corner walks. Measured around 0.45ms for the full set. */
	constexpr float BuildRouteMaxDuration = 2.0f;

	/**
	 * Hull building dominates: each of the two sweeps emits a prism per path segment, and every prism computes its
	 * bounds and convexity up front so the intersection tests that follow can take their fast paths. Measured
	 * around 7.8ms for the full set, with headroom here for slower machines.
	 */
	constexpr float SweepMaxDuration = 20.0f;

	static const FVector2D SocketUnitSize = FVector2D(50.f, 50.f);

	static FNWorldAssemblyJunctionConnectorSettings MakeSettings()
	{
		FNWorldAssemblyJunctionConnectorSettings Settings;
		Settings.MaximumRange = 5000.f;
		Settings.MaximumSplineLength = 5000.f;
		Settings.SplineRadius = 200.f;
		Settings.SampleStep = 50.f;
		Settings.TangentScale = 0.5f;
		return Settings;
	}

	/** A junction at Location facing Yaw, sized to the project default socket. */
	static FNCellJunctionDetails MakeJunction(const FVector& Location, const float Yaw)
	{
		FNCellJunctionDetails Junction;
		Junction.SocketSize = FIntVector2(2, 4);
		Junction.WorldLocation = Location;
		Junction.WorldRotation = FRotator(0.f, Yaw, 0.f);
		return Junction;
	}
}

class FNJunctionConnectorSolverPerfTests
{
public:
	// Measures routing a full candidate set: control points, sampling, corner pairing and the four corner curves.
	static void BuildRoute()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

		const FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();

		// Spread the pairs out so no two share a socket position, matching how a real layout presents them.
		TArray<TPair<FNCellJunctionDetails, FNCellJunctionDetails>> Pairs;
		Pairs.Reserve(RouteCount);
		for (int32 i = 0; i < RouteCount; i++)
		{
			const FVector Base(0.0, i * 500.0, 0.0);
			Pairs.Add({
				MakeJunction(Base, 180.f),
				MakeJunction(Base + FVector(PairDistance, 0.0, 0.0), 0.f)
			});
		}

		FNJunctionConnectorRoute Route;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNJunctionConnectorSolverPerfTests_BuildRoute,
				NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolverHarness::BuildRouteMaxDuration)
			for (int32 i = 0; i < RouteCount; i++)
			{
				FNJunctionConnectorSolver::BuildRoute(Pairs[i].Key, Pairs[i].Value, SocketUnitSize, Settings, nullptr, Route);
			}
			NTestTimer.ManualStop();
		}
	}

	// Measures building the swept hulls for a routed pair — the dominant per-candidate cost, since every hull then
	// gets tested against world geometry, placed cells and previously accepted connectors.
	static void BuildHulls()
	{
		using namespace NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

		const FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();

		FNJunctionConnectorRoute Route;
		if (!FNJunctionConnectorSolver::BuildRoute(
			MakeJunction(FVector::ZeroVector, 180.f),
			MakeJunction(FVector(PairDistance, 0.0, 0.0), 0.f),
			SocketUnitSize, Settings, nullptr, Route))
		{
			return;
		}

		TArray<FNRawMesh> RadiusHulls;
		TArray<FNRawMesh> CornerHulls;

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNJunctionConnectorSolverPerfTests_BuildHulls,
				NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolverHarness::SweepMaxDuration)
			for (int32 i = 0; i < RouteCount; i++)
			{
				FNJunctionConnectorSolver::BuildRadiusHulls(Route, Settings.SplineRadius, RadiusHulls);
				FNJunctionConnectorSolver::BuildCornerHulls(Route, CornerHulls);
			}
			NTestTimer.ManualStop();
		}
	}
};

N_TEST_PERF(FNJunctionConnectorSolverPerfTests_BuildRoute,
	"NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolver::BuildRoute",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNJunctionConnectorSolverPerfTests::BuildRoute));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

N_TEST_PERF(FNJunctionConnectorSolverPerfTests_BuildHulls,
	"NEXUS::PerfTests::NWorldAssembly::FNJunctionConnectorSolver::BuildHulls",
	N_TEST_CONTEXT_ANYWHERE)
{
	N_TESTS_PERF_START_LATENT_TEST
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand(&FNJunctionConnectorSolverPerfTests::BuildHulls));
	N_TESTS_PERF_FINISH_LATENT_TEST
}

#endif //WITH_TESTS
