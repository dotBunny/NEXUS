// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyUtils.h"
#include "Assembly/NJunctionConnectorSolver.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness
{
	/** World size of one socket grid unit; matches the project default so socket maths reads the same as in-editor. */
	static const FVector2D SocketUnitSize = FVector2D(50.f, 50.f);

	/** A non-square socket, so corner-pairing tests exercise the dimension gate rather than the square special case. */
	static const FIntVector2 SocketUnits = FIntVector2(2, 4);

	/** Settings roomy enough that only the constraint a given test targets can reject a route. */
	static FNWorldAssemblyJunctionConnectorSettings MakeSettings()
	{
		FNWorldAssemblyJunctionConnectorSettings Settings;
		Settings.MaximumRange = 100000.f;
		Settings.MaximumSplineLength = 100000.f;
		Settings.SplineRadius = 50.f;
		Settings.SampleStep = 50.f;
		Settings.TangentScale = 0.5f;
		Settings.MaximumAvoidanceAttempts = 16;
		Settings.AvoidanceOffsetStep = 200.f;
		Settings.EndpointExclusion = 100.f;
		// Off by default here so the tests below stay about the constraint each one targets; the turn-radius tests
		// opt back in explicitly. The always-on fold rejection still applies.
		Settings.MinimumTurnRadiusScale = 0.f;
		return Settings;
	}

	/**
	 * Build a route at the settings' own tangent scale.
	 * @return true when the solver accepted it; the tests that care *why* one was rejected call the solver directly.
	 */
	static bool BuildRoute(const FNCellJunctionDetails& Start, const FNCellJunctionDetails& End,
		const FVector2D& UnitSize, const FNWorldAssemblyJunctionConnectorSettings& Settings,
		const FVector* MidPoint, FNJunctionConnectorRoute& OutRoute)
	{
		return FNJunctionConnectorSolver::BuildRoute(Start, End, UnitSize, Settings, MidPoint,
			Settings.TangentScale, OutRoute) == ENJunctionConnectorRouteResult::Success;
	}

	/**
	 * A junction at Location whose rotation faces Yaw degrees.
	 * @note Rotation faces *into* the owning cell, so a junction opens onto the opposite direction — the two
	 *       facing-each-other helpers below encode that so no test has to re-derive it.
	 */
	static FNCellJunctionDetails MakeJunction(const FVector& Location, const float Yaw)
	{
		FNCellJunctionDetails Junction;
		Junction.SocketSize = SocketUnits;
		Junction.WorldLocation = Location;
		Junction.WorldRotation = FRotator(0.f, Yaw, 0.f);
		return Junction;
	}

	/** A junction at the origin opening along +X (so its rotation, which faces inward, points along -X). */
	static FNCellJunctionDetails MakeStartJunction()
	{
		return MakeJunction(FVector::ZeroVector, 180.f);
	}

	/** A junction Distance along +X opening back along -X, i.e. facing the start junction across open space. */
	static FNCellJunctionDetails MakeEndJunction(const double Distance)
	{
		return MakeJunction(FVector(Distance, 0.0, 0.0), 0.f);
	}
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_Direction_OutwardIsOppositeTheRotation,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Direction::OutwardIsOppositeTheRotation",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A junction's rotation faces into the cell that owns it, so the direction it opens onto is the negated forward.
	// Every route leaves and arrives along this, so an inverted sign would send every connector into its own cell.
	const FNCellJunctionDetails Junction = NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness::MakeJunction(
		FVector::ZeroVector, 0.f);

	const FVector Outward = FNWorldAssemblyUtils::GetJunctionOutwardDirection(Junction);
	CHECK_MESSAGE(TEXT("A junction must open onto the opposite of its rotation's forward vector."),
		Outward.Equals(-Junction.WorldRotation.Vector()))
	CHECK_MESSAGE(TEXT("A junction rotated to face +X must open onto -X."), Outward.Equals(FVector(-1.0, 0.0, 0.0)))
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_Route_FacingJunctionsRouteStraight,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Route::FacingJunctionsRouteStraight",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two junctions opening onto each other across clear space should route almost straight between them. This is
	// the regression guard for the facing convention: were the outward direction inverted, the same pair would need
	// a U-turn out of both cells and the route would be several times the chord.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	constexpr double Distance = 500.0;
	const FNCellJunctionDetails Start = MakeStartJunction();
	const FNCellJunctionDetails End = MakeEndJunction(Distance);

	FNJunctionConnectorRoute Route;
	CHECK_MESSAGE(TEXT("A clear, facing pair must produce a route."),
		BuildRoute(Start, End, SocketUnitSize, MakeSettings(), nullptr, Route))

	CHECK_MESSAGE(TEXT("A straight route must be within a hair of the chord length."),
		FMath::IsNearlyEqual(static_cast<double>(Route.Path.Center.Length), Distance, 1.0))
	CHECK_MESSAGE(TEXT("The route must start on the start socket."),
		Route.Path.Center.Points[0].Equals(Start.WorldLocation))
	CHECK_MESSAGE(TEXT("The route must end on the end socket."),
		Route.Path.Center.Points.Last().Equals(End.WorldLocation))
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_Route_LeavesAndArrivesAlongTheSocketNormals,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Route::LeavesAndArrivesAlongTheSocketNormals",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A connector welds to both openings, so the route has to meet each one square on: leaving along the direction
	// the start socket opens onto, and arriving travelling into the cell that owns the end socket. This is the
	// invariant the facing convention exists to serve, and it must hold whichever way the pair happens to point.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	auto CheckRoute = [this](const FNCellJunctionDetails& Start, const FNCellJunctionDetails& End, const TCHAR* Label)
	{
		FNJunctionConnectorRoute Route;
		if (!BuildRoute(Start, End, SocketUnitSize, MakeSettings(), nullptr, Route))
		{
			ADD_ERROR(FString::Printf(TEXT("Expected %s to route."), Label));
			return;
		}

		const FVector StartOutward = FNWorldAssemblyUtils::GetJunctionOutwardDirection(Start);
		const FVector EndInward = -FNWorldAssemblyUtils::GetJunctionOutwardDirection(End);

		// The terminal frames are pinned to the socket normals rather than derived from the sampled polyline, which
		// is what lets the corner curves land square on both openings however hard the route turns in between.
		CHECK_MESSAGE(TEXT("The first frame must face along the direction the start socket opens onto."),
			Route.Frames[0].Tangent.Equals(StartOutward))
		CHECK_MESSAGE(TEXT("The last frame must face into the cell that owns the end socket."),
			Route.Frames.Last().Tangent.Equals(EndInward))

		// The sampled polyline only approximates that — a hard turn puts real curvature inside the first step — so it
		// is held to the weaker claim that it at least sets off the right way.
		const FVector FirstStep = (Route.Path.Center.Points[1] - Route.Path.Center.Points[0]).GetSafeNormal();
		const int32 LastIndex = Route.Path.Center.Points.Num() - 1;
		const FVector LastStep = (Route.Path.Center.Points[LastIndex] - Route.Path.Center.Points[LastIndex - 1]).GetSafeNormal();

		CHECK_MESSAGE(TEXT("A route's first step must set off out of the start socket, not back into its cell."),
			FVector::DotProduct(FirstStep, StartOutward) > 0.0)
		CHECK_MESSAGE(TEXT("A route's last step must arrive into the end socket, not back out of it."),
			FVector::DotProduct(LastStep, EndInward) > 0.0)
	};

	// Facing each other across open space: the ordinary case, and the only one that produces a buildable route — see
	// the fold test for why a back-to-back pair no longer does.
	CheckRoute(MakeStartJunction(), MakeEndJunction(500.0), TEXT("a facing pair"));
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_Route_LengthBudgetRejectsALongRoute,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Route::LengthBudgetRejectsALongRoute",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A pair well inside the matching range can still be too far apart to bridge; the spline budget is the gate that
	// enforces that, independently of range.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MaximumSplineLength = 400.f;

	FNJunctionConnectorRoute Route;
	CHECK_MESSAGE(TEXT("A pair inside the budget must route."),
		BuildRoute(MakeStartJunction(), MakeEndJunction(300.0), SocketUnitSize, Settings, nullptr, Route))
	CHECK_FALSE_MESSAGE(TEXT("A pair beyond the budget must be rejected even though it is well within range."),
		BuildRoute(MakeStartJunction(), MakeEndJunction(900.0), SocketUnitSize, Settings, nullptr, Route))
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_Corners_MatedPairResolvesToTheMirrorPairing,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Corners::MatedPairResolvesToTheMirrorPairing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two junctions facing each other see one another's socket from behind, so the join that does not fold reverses
	// the corner winding — for an unrolled pair that is the straight mirror, corner i to corner 3-i.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	const TArray<FVector> StartCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(MakeStartJunction(), SocketUnitSize);
	const TArray<FVector> EndCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(MakeEndJunction(500.0), SocketUnitSize);

	const TStaticArray<int32, 4> Pairing = FNJunctionConnectorSolver::ResolveCornerPairing(StartCorners, EndCorners, false);
	for (int32 i = 0; i < 4; i++)
	{
		CHECK_EQUALS("A facing, unrolled pair must resolve to the mirror pairing.", Pairing[i], 3 - i)
	}
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_Corners_PairingIsAPermutation,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Corners::PairingIsAPermutation",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Whatever the relative roll, every start corner must claim a distinct end corner — a pairing that doubled up
	// would collapse two edges of the tube onto one.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNCellJunctionDetails End = MakeEndJunction(500.0);
	End.WorldRotation = FRotator(0.f, 0.f, 30.f);

	const TArray<FVector> StartCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(MakeStartJunction(), SocketUnitSize);
	const TArray<FVector> EndCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(End, SocketUnitSize);

	const TStaticArray<int32, 4> Pairing = FNJunctionConnectorSolver::ResolveCornerPairing(StartCorners, EndCorners, false);

	TSet<int32> Claimed;
	for (int32 i = 0; i < 4; i++)
	{
		CHECK_MESSAGE(TEXT("A corner pairing must index a real corner."), Pairing[i] >= 0 && Pairing[i] < 4)
		Claimed.Add(Pairing[i]);
	}
	CHECK_EQUALS("Every start corner must pair with a distinct end corner.", Claimed.Num(), 4)
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_Corners_CurvesTerminateOnTheAuthoredSocketCorners,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Corners::CurvesTerminateOnTheAuthoredSocketCorners",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A connector welds geometry to both openings, so the corner curves have to land exactly on the sockets' own
	// vertices — not merely near them.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	const FNCellJunctionDetails Start = MakeStartJunction();
	const FNCellJunctionDetails End = MakeEndJunction(500.0);

	FNJunctionConnectorRoute Route;
	if (!BuildRoute(Start, End, SocketUnitSize, MakeSettings(), nullptr, Route))
	{
		ADD_ERROR("Expected a clear, facing pair to route.");
		return;
	}

	CHECK_EQUALS("A route must carry one curve per socket corner.", Route.Path.Corners.Num(), 4)

	const TArray<FVector> StartCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(Start, SocketUnitSize);
	const TArray<FVector> EndCorners = FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(End, SocketUnitSize);
	const TStaticArray<int32, 4> Pairing = FNJunctionConnectorSolver::ResolveCornerPairing(StartCorners, EndCorners, false);

	for (int32 i = 0; i < 4; i++)
	{
		const TArray<FVector>& Points = Route.Path.Corners[i].Points;
		CHECK_MESSAGE(TEXT("A corner curve must start on its start socket corner."), Points[0].Equals(StartCorners[i]))
		CHECK_MESSAGE(TEXT("A corner curve must end on the end socket corner it was paired with."),
			Points.Last().Equals(EndCorners[Pairing[i]]))
	}
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_Corners_CurvesTrackTheCenterSampleCount,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Corners::CurvesTrackTheCenterSampleCount",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Consumers build quads across the four corner arrays by index, so the arrays have to stay in lockstep with each
	// other and with the center curve.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNJunctionConnectorRoute Route;
	if (!BuildRoute(MakeStartJunction(), MakeEndJunction(500.0), SocketUnitSize, MakeSettings(), nullptr, Route))
	{
		ADD_ERROR("Expected a clear, facing pair to route.");
		return;
	}

	const int32 CenterCount = Route.Path.Center.Points.Num();
	CHECK_MESSAGE(TEXT("A route must be sampled into at least one segment."), CenterCount > 1)
	for (int32 i = 0; i < 4; i++)
	{
		CHECK_EQUALS("Each corner curve must carry one sample per center sample.", Route.Path.Corners[i].Points.Num(), CenterCount)
	}
	CHECK_EQUALS("The frame list must carry one entry per center sample.", Route.Frames.Num(), CenterCount)
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_Route_IsDeterministic,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Route::IsDeterministic",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Nothing in routing consults a random stream, so the same pair must produce a bit-identical path every run —
	// which is what lets a seed reproduce a whole layout, connectors included.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	const FNCellJunctionDetails Start = MakeStartJunction();
	const FNCellJunctionDetails End = MakeEndJunction(700.0);

	FNJunctionConnectorRoute First;
	FNJunctionConnectorRoute Second;
	if (!BuildRoute(Start, End, SocketUnitSize, MakeSettings(), nullptr, First)
		|| !BuildRoute(Start, End, SocketUnitSize, MakeSettings(), nullptr, Second))
	{
		ADD_ERROR("Expected a clear, facing pair to route.");
		return;
	}

	CHECK_EQUALS("Two runs must sample the center curve identically.", First.Path.Center.Points.Num(), Second.Path.Center.Points.Num())
	for (int32 i = 0; i < First.Path.Center.Points.Num(); i++)
	{
		if (First.Path.Center.Points[i] != Second.Path.Center.Points[i])
		{
			ADD_ERROR("Two runs of the same pair produced different center samples.");
			return;
		}
	}

	for (int32 Corner = 0; Corner < 4; Corner++)
	{
		for (int32 i = 0; i < First.Path.Corners[Corner].Points.Num(); i++)
		{
			if (First.Path.Corners[Corner].Points[i] != Second.Path.Corners[Corner].Points[i])
			{
				ADD_ERROR("Two runs of the same pair produced different corner samples.");
				return;
			}
		}
	}
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_Avoidance_MidPointsFanOutNearestFirst,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Avoidance::MidPointsFanOutNearestFirst",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Detours are tried in order, so the ordering decides which one is accepted. Emitting them nearest-first is what
	// makes the accepted route the least-deviating one that clears.
	const FVector Start = FVector::ZeroVector;
	const FVector End = FVector(1000.0, 0.0, 0.0);
	constexpr float OffsetStep = 200.f;

	TArray<FVector> MidPoints;
	FNJunctionConnectorSolver::BuildAvoidanceMidPoints(Start, End, OffsetStep, 16, MidPoints);

	CHECK_EQUALS("The requested number of detour candidates must be emitted.", MidPoints.Num(), 16)

	const FVector ChordMidPoint = (Start + End) * 0.5;
	double PreviousDistance = 0.0;
	for (int32 i = 0; i < MidPoints.Num(); i++)
	{
		const double Distance = FVector::Dist(MidPoints[i], ChordMidPoint);
		CHECK_MESSAGE(TEXT("A detour must actually leave the direct path."), Distance > UE_KINDA_SMALL_NUMBER)
		CHECK_MESSAGE(TEXT("Detour candidates must never move closer to the direct path than an earlier one."),
			Distance >= PreviousDistance - UE_KINDA_SMALL_NUMBER)
		PreviousDistance = Distance;
	}

	// The first ring sits one step out; the second ring only begins once the first is exhausted.
	CHECK_MESSAGE(TEXT("The first detour must sit exactly one offset step off the direct path."),
		FMath::IsNearlyEqual(FVector::Dist(MidPoints[0], ChordMidPoint), static_cast<double>(OffsetStep), 0.01))
	CHECK_MESSAGE(TEXT("The ninth detour must open the second ring, two offset steps out."),
		FMath::IsNearlyEqual(FVector::Dist(MidPoints[8], ChordMidPoint), static_cast<double>(OffsetStep) * 2.0, 0.01))
}

N_TEST_MEDIUM(FNJunctionConnectorSolverTests_Avoidance_MidPointRoutesAroundTheDirectPath,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Avoidance::MidPointRoutesAroundTheDirectPath",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A detour has to actually bend the route away from the straight line, and pay for it in length — otherwise
	// retrying with one would test the same geometry that already failed.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	const FNCellJunctionDetails Start = MakeStartJunction();
	const FNCellJunctionDetails End = MakeEndJunction(500.0);

	FNJunctionConnectorRoute Direct;
	FNJunctionConnectorRoute Detoured;
	// Kept gentle enough to stay buildable: a harder swerve off a chord this short folds the connector's geometry at
	// the socket, which the fold check rejects outright.
	const FVector MidPoint = FVector(250.0, 150.0, 0.0);
	if (!BuildRoute(Start, End, SocketUnitSize, MakeSettings(), nullptr, Direct)
		|| !BuildRoute(Start, End, SocketUnitSize, MakeSettings(), &MidPoint, Detoured))
	{
		ADD_ERROR("Expected both the direct and detoured routes to build.");
		return;
	}

	CHECK_MESSAGE(TEXT("A detoured route must be longer than the direct one."),
		Detoured.Path.Center.Length > Direct.Path.Center.Length)
	CHECK_MESSAGE(TEXT("A detoured route must still terminate on both sockets."),
		Detoured.Path.Center.Points[0].Equals(Start.WorldLocation)
		&& Detoured.Path.Center.Points.Last().Equals(End.WorldLocation))

	// The detour is only useful if it clears the space the direct route occupied.
	double MaximumOffset = 0.0;
	for (const FVector& Point : Detoured.Path.Center.Points)
	{
		MaximumOffset = FMath::Max(MaximumOffset, FMath::Abs(Point.Y));
	}
	CHECK_MESSAGE(TEXT("A detoured route must bend meaningfully off the direct path."), MaximumOffset > 100.0)
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_TurnRadius_StraighteningRecoversATooTightRoute,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::StraighteningRecoversATooTightRoute",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The recovery the pass relies on: the same pair and the same detour, rejected as too tight at one tangent scale,
	// builds at a wider one. Without this a hard swerve would simply be lost rather than opened up.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MinimumTurnRadiusScale = 0.f;

	const FVector MidPoint = FVector(250.0, 300.0, 0.0);
	FNJunctionConnectorRoute Route;

	CHECK_EQUALS("The detour must be too tight to build at a short tangent scale.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(500.0),
			SocketUnitSize, Settings, &MidPoint, 0.5f, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::TooTight))

	CHECK_EQUALS("The same detour must build once the tangents are widened.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(500.0),
			SocketUnitSize, Settings, &MidPoint, 1.0f, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::Success))
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_Sweep_HullsSpanEverySegment,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Sweep::HullsSpanEverySegment",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The swept hulls are what the route is cleared against, so a missing segment is a hole in the collision test.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNJunctionConnectorRoute Route;
	if (!BuildRoute(MakeStartJunction(), MakeEndJunction(500.0), SocketUnitSize, MakeSettings(), nullptr, Route))
	{
		ADD_ERROR("Expected a clear, facing pair to route.");
		return;
	}

	const int32 SegmentCount = Route.Path.Center.Points.Num() - 1;

	TArray<FNRawMesh> RadiusHulls;
	FNJunctionConnectorSolver::BuildRadiusHulls(Route, 50.f, RadiusHulls);
	CHECK_EQUALS("The radius sweep must emit one hull per path segment.", RadiusHulls.Num(), SegmentCount)

	TArray<FNRawMesh> CornerHulls;
	FNJunctionConnectorSolver::BuildCornerHulls(Route, CornerHulls);
	CHECK_EQUALS("The corner sweep must emit one hull per path segment.", CornerHulls.Num(), SegmentCount)
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_Sweep_CornerHullsDetectBlockingGeometry,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Sweep::CornerHullsDetectBlockingGeometry",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The whole pass rests on the swept hulls reporting an obstruction that sits across the route — and on them not
	// reporting one that sits well clear of it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNJunctionConnectorRoute Route;
	if (!BuildRoute(MakeStartJunction(), MakeEndJunction(500.0), SocketUnitSize, MakeSettings(), nullptr, Route))
	{
		ADD_ERROR("Expected a clear, facing pair to route.");
		return;
	}

	TArray<FNRawMesh> CornerHulls;
	FNJunctionConnectorSolver::BuildCornerHulls(Route, CornerHulls);
	if (CornerHulls.IsEmpty())
	{
		ADD_ERROR("Expected the corner sweep to emit hulls.");
		return;
	}

	// A slab straddling the midpoint of the route.
	const FNRawMesh Blocker = FNRawMeshUtils::MakeBoxHull(FBox(FVector(200.0, -200.0, -200.0), FVector(300.0, 200.0, 200.0)));
	bool bBlocked = false;
	for (const FNRawMesh& Hull : CornerHulls)
	{
		if (FNRawMeshUtils::DoesIntersect(Hull, Blocker))
		{
			bBlocked = true;
			break;
		}
	}
	CHECK_MESSAGE(TEXT("A slab across the route must be detected by the swept hulls."), bBlocked)

	// The same slab moved well off to the side.
	const FNRawMesh Clear = FNRawMeshUtils::MakeBoxHull(FBox(FVector(200.0, 2000.0, -200.0), FVector(300.0, 2400.0, 200.0)));
	bool bFalsePositive = false;
	for (const FNRawMesh& Hull : CornerHulls)
	{
		if (FNRawMeshUtils::DoesIntersect(Hull, Clear))
		{
			bFalsePositive = true;
			break;
		}
	}
	CHECK_FALSE_MESSAGE(TEXT("Geometry well clear of the route must not be reported as blocking."), bFalsePositive)
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_TurnRadius_GentleRoutesPassWhereHairpinsFail,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::GentleRoutesPassWhereHairpinsFail",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The point of the limit: on identical settings, a route that barely bends is accepted and one forced into a
	// hairpin is not.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MinimumTurnRadiusScale = 2.f;

	FNJunctionConnectorRoute Route;
	CHECK_MESSAGE(TEXT("A near-straight route must clear the turn-radius limit."),
		FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(800.0), SocketUnitSize, Settings,
			nullptr, Settings.TangentScale, Route) == ENJunctionConnectorRouteResult::Success)

	// A detour yanked hard off a short chord: the same pair, bent sharply.
	const FVector Hairpin = FVector(150.0, 900.0, 0.0);
	CHECK_EQUALS("A hairpin route must be rejected as too tight.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(300.0),
			SocketUnitSize, Settings, &Hairpin, Settings.TangentScale, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::TooTight))
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_TurnRadius_MeasuredAgainstTheSocketExtentOfTheTurnPlane,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::MeasuredAgainstTheSocketExtentOfTheTurnPlane",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Turns are not always horizontal, and the socket is not square: the default is twice as tall as it is wide. So
	// the same turn radius leaves twice as much room to spare bending sideways as it does bending vertically, and the
	// reported scale has to reflect that rather than assuming a yaw turn.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	const FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	const FVector2D SocketWorldSize = FNWorldAssemblyUtils::GetWorldSize2D(SocketUnits, SocketUnitSize);

	// Confirm the fixture really is non-square, or the comparison below proves nothing.
	CHECK_MESSAGE(TEXT("The socket fixture must be taller than it is wide for this test to mean anything."),
		SocketWorldSize.Y > SocketWorldSize.X)

	// Gentle enough that the turn clears the socket's *height* extent, not just its width — the vertical twin below
	// has only half the clearance, and a harder bend would fold it before the comparison could be made.
	constexpr double Distance = 600.0;
	constexpr double Offset = 150.0;
	constexpr float TangentScale = 1.0f;

	// The same bend, once swung sideways and once swung upward.
	const FVector HorizontalMid = FVector(Distance * 0.5, Offset, 0.0);
	const FVector VerticalMid = FVector(Distance * 0.5, 0.0, Offset);

	FNJunctionConnectorRoute Horizontal;
	FNJunctionConnectorRoute Vertical;
	if (FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(Distance), SocketUnitSize, Settings,
			&HorizontalMid, TangentScale, Horizontal) != ENJunctionConnectorRouteResult::Success
		|| FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(Distance), SocketUnitSize, Settings,
			&VerticalMid, TangentScale, Vertical) != ENJunctionConnectorRouteResult::Success)
	{
		ADD_ERROR("Expected both the horizontal and vertical detours to build with the radius check disabled.");
		return;
	}

	const float HorizontalScale = FNJunctionConnectorSolver::GetMinimumTurnRadiusScale(Horizontal, SocketWorldSize);
	const float VerticalScale = FNJunctionConnectorSolver::GetMinimumTurnRadiusScale(Vertical, SocketWorldSize);

	CHECK_MESSAGE(TEXT("Both mirrored detours must report a measurable turn."),
		HorizontalScale < MAX_flt && VerticalScale < MAX_flt)

	// Geometrically the two curves are the same shape, so any difference in the reported scale comes entirely from
	// which socket extent the turn has to clear. The taller extent must yield the smaller ratio.
	CHECK_MESSAGE(TEXT("A vertical turn must report less room to spare than the same turn taken sideways, because the socket is taller than it is wide."),
		VerticalScale < HorizontalScale)

	// And the ratio between them must track the socket's aspect, not some arbitrary factor.
	const float ExpectedRatio = static_cast<float>(SocketWorldSize.X / SocketWorldSize.Y);
	CHECK_MESSAGE(TEXT("The two scales must differ by the socket's own width-to-height ratio."),
		FMath::IsNearlyEqual(VerticalScale / HorizontalScale, ExpectedRatio, 0.05f))
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_TurnRadius_VerticalTurnIsRejectedWhereTheSameHorizontalOneIsNot,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::VerticalTurnIsRejectedWhereTheSameHorizontalOneIsNot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The consequence of the above, at the gate: there is a band of curvature a sideways turn survives and the
	// identical vertical turn does not, because the connector's geometry is taller than it is wide.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	// Gentle enough that the turn clears the socket's *height* extent, not just its width — the vertical twin below
	// has only half the clearance, and a harder bend would fold it before the comparison could be made.
	constexpr double Distance = 600.0;
	constexpr double Offset = 150.0;
	constexpr float TangentScale = 1.0f;
	const FVector HorizontalMid = FVector(Distance * 0.5, Offset, 0.0);
	const FVector VerticalMid = FVector(Distance * 0.5, 0.0, Offset);

	const FVector2D SocketWorldSize = FNWorldAssemblyUtils::GetWorldSize2D(SocketUnits, SocketUnitSize);

	// Measure the horizontal route, then set the limit just under it — the vertical twin has proportionally less
	// clearance, so it must fall the other side of that line.
	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	FNJunctionConnectorRoute Route;
	if (FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(Distance), SocketUnitSize, Settings,
		&HorizontalMid, TangentScale, Route) != ENJunctionConnectorRouteResult::Success)
	{
		ADD_ERROR("Expected the horizontal detour to build with the radius check disabled.");
		return;
	}
	Settings.MinimumTurnRadiusScale = FNJunctionConnectorSolver::GetMinimumTurnRadiusScale(Route, SocketWorldSize) * 0.95f;

	CHECK_EQUALS("The horizontal turn must clear a limit set just below its own measured radius.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(Distance),
			SocketUnitSize, Settings, &HorizontalMid, TangentScale, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::Success))

	CHECK_EQUALS("The identical vertical turn must be rejected at that same limit.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(Distance),
			SocketUnitSize, Settings, &VerticalMid, TangentScale, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::TooTight))
}

N_TEST_CRITICAL(FNJunctionConnectorSolverTests_TurnRadius_FoldedRoutesAreRejectedWithTheCheckDisabled,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::FoldedRoutesAreRejectedWithTheCheckDisabled",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A route bent tighter than the socket is wide has its inner wall pass through itself, and no connector could
	// build geometry along it. That is a validity failure, so it must be rejected even with the tunable limit turned
	// all the way off.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MinimumTurnRadiusScale = 0.f;

	// A violent detour off a very short chord, well inside the socket's own half-extent.
	const FVector Fold = FVector(20.0, 1200.0, 0.0);

	FNJunctionConnectorRoute Route;
	CHECK_EQUALS("A folded route must be rejected even with the turn-radius limit disabled.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(120.0),
			SocketUnitSize, Settings, &Fold, Settings.TangentScale, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::TooTight))

	// And a route that is merely gentle must not trip the fold detector.
	if (!BuildRoute(MakeStartJunction(), MakeEndJunction(800.0), SocketUnitSize, Settings, nullptr, Route))
	{
		ADD_ERROR("Expected a near-straight route to build.");
		return;
	}
	CHECK_FALSE_MESSAGE(TEXT("A gentle route must not be reported as folded."), FNJunctionConnectorSolver::DoesRouteFold(Route))
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_TurnRadius_BackToBackPairsFoldAtEveryTangentScale,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::BackToBackPairsFoldAtEveryTangentScale",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Nothing gates candidate pairs on whether their junctions face each other, so a back-to-back pair reaches the
	// solver. Meeting both sockets square on then forces the route to double back out of each opening, and no tangent
	// scale opens that turn far enough to clear the socket — so the fold check is what actually rejects these.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MinimumTurnRadiusScale = 0.f;

	// Sockets side by side, each opening away from the other.
	const FNCellJunctionDetails Start = MakeJunction(FVector::ZeroVector, 0.f);
	const FNCellJunctionDetails End = MakeJunction(FVector(0.0, 500.0, 0.0), 180.f);

	FNJunctionConnectorRoute Route;
	for (const float TangentScale : { 0.5f, 1.0f, 1.5f, 2.0f })
	{
		CHECK_EQUALS("A back-to-back pair must be rejected as too tight at every tangent scale.",
			static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(Start, End, SocketUnitSize, Settings,
				nullptr, TangentScale, Route)),
			static_cast<int32>(ENJunctionConnectorRouteResult::TooTight))
	}
}

N_TEST_HIGH(FNJunctionConnectorSolverTests_TurnRadius_LengthRisesWithEveryStraighteningStep,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::LengthRisesWithEveryStraighteningStep",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The straightening retry abandons a detour the moment one of its steps blows the length budget, on the grounds
	// that every later step is longer still. That shortcut is only safe if length rises monotonically with the tangent
	// scale — so this pins it.
	//
	// Note that curvature deliberately is *not* asserted to improve monotonically: longer tangents open a turn up to
	// a point and then overshoot into a tighter one, which is exactly why the retry tries every step instead of
	// jumping to the widest.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MinimumTurnRadiusScale = 0.f;

	// Two facing sockets offset sideways, producing a gentle S. Gentle enough to stay buildable across the whole
	// tangent range, which a detour is not — the point here is the length trend, not the turn.
	const FNCellJunctionDetails Start = MakeStartJunction();
	const FNCellJunctionDetails End = MakeJunction(FVector(800.0, 100.0, 0.0), 0.f);

	float PreviousLength = 0.f;
	for (const float TangentScale : { 0.25f, 0.5f, 1.0f, 1.5f, 2.0f })
	{
		FNJunctionConnectorRoute Route;
		if (FNJunctionConnectorSolver::BuildRoute(Start, End, SocketUnitSize, Settings, nullptr, TangentScale, Route)
			!= ENJunctionConnectorRouteResult::Success)
		{
			ADD_ERROR(FString::Printf(TEXT("Expected the route to build at tangent scale %.2f."), TangentScale));
			return;
		}

		CHECK_MESSAGE(TEXT("Each straightening step must produce a longer route than the one before it."),
			Route.Path.Center.Length > PreviousLength)
		PreviousLength = Route.Path.Center.Length;
	}
}

N_TEST_MEDIUM(FNJunctionConnectorSolverTests_TurnRadius_StraightRoutesReportNoTurn,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::TurnRadius::StraightRoutesReportNoTurn",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A route with no curvature to measure must report an unbounded scale rather than a degenerate zero, or the gate
	// would reject exactly the routes that are most obviously fine.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNWorldAssemblyJunctionConnectorSettings Settings = MakeSettings();
	Settings.MinimumTurnRadiusScale = 10.f;

	FNJunctionConnectorRoute Route;
	CHECK_EQUALS("A dead-straight route must clear even a demanding turn-radius limit.",
		static_cast<int32>(FNJunctionConnectorSolver::BuildRoute(MakeStartJunction(), MakeEndJunction(500.0),
			SocketUnitSize, Settings, nullptr, Settings.TangentScale, Route)),
		static_cast<int32>(ENJunctionConnectorRouteResult::Success))
}

N_TEST_MEDIUM(FNJunctionConnectorSolverTests_Route_CoincidentSocketsAreRejected,
	"NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolver::Route::CoincidentSocketsAreRejected",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two sockets at the same point give no direction to route along and nothing meaningful to hand a connector.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionConnectorSolverHarness;

	FNJunctionConnectorRoute Route;
	CHECK_FALSE_MESSAGE(TEXT("Coincident sockets must not produce a route."),
		BuildRoute(MakeStartJunction(), MakeEndJunction(0.0), SocketUnitSize, MakeSettings(), nullptr, Route))
}

#endif //WITH_TESTS
