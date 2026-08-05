// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyUtils.h"
#include "Cell/NCellJunctionDetails.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness
{
	/** The shipped FNWorldAssemblyJunctionConnectorSettings limits, restated so a default change is caught here. */
	constexpr float DefaultFacing = 90.f;
	constexpr float DefaultApproach = 90.f;
	constexpr float DefaultElevation = 45.f;

	/** Every limit wide open — the configuration that opts the whole gate out. */
	constexpr float NoLimit = 180.f;

	/**
	 * A junction at Location whose socket opens onto Outward.
	 *
	 * Built from the outward direction rather than a rotator because that is what every one of these tests is
	 * actually about, and because a junction's stored rotation faces *into* its own cell — writing the rotators by
	 * hand would put a negation between each fixture and the case it is meant to describe.
	 * @param Location World-space center of the socket.
	 * @param Outward The direction the socket opens onto, away from the cell that owns it.
	 * @return The junction details, with no per-junction override set.
	 */
	static FNCellJunctionDetails MakeJunctionFacing(const FVector& Location, const FVector& Outward)
	{
		FNCellJunctionDetails Details;
		Details.WorldLocation = Location;
		Details.WorldRotation = (-Outward.GetSafeNormal()).Rotation();
		return Details;
	}

	/** @return AreJunctionsWithinConnectionAngles under the shipped default limits. */
	static bool IsWithinDefaults(const FNCellJunctionDetails& A, const FNCellJunctionDetails& B)
	{
		return FNWorldAssemblyUtils::AreJunctionsWithinConnectionAngles(A, B,
			DefaultFacing, DefaultApproach, DefaultElevation);
	}

	/** A ceiling hatch and a wall door, positioned so only the elevation difference can separate them. */
	static void MakeHatchAndWallDoor(FNCellJunctionDetails& OutHatch, FNCellJunctionDetails& OutWallDoor)
	{
		// Facing lands on exactly 90 (up against sideways) and both approach angles on 45, so every limit but
		// elevation is satisfied. Elevation differs by the full 90.
		OutHatch = MakeJunctionFacing(FVector::ZeroVector, FVector::UpVector);
		OutWallDoor = MakeJunctionFacing(FVector(0.0, 500.0, 500.0), -FVector::RightVector);
	}
}

//
// AreJunctionsWithinConnectionAngles — the orientation gate FNConnectJunctionsTask builds candidate pairs on.
//

N_TEST_CRITICAL(FNWorldAssemblyUtilsTests_ConnectionAngles_AcceptsAHeadOnPair,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::AcceptsAHeadOnPair",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The load-bearing case: two sockets squarely facing each other across clear space is the pairing the whole pass
	// exists to make. No configuration of the limits that rejects this one is defensible.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails A = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	const FNCellJunctionDetails B = MakeJunctionFacing(FVector(1000.0, 0.0, 0.0), -FVector::ForwardVector);

	CHECK_MESSAGE(TEXT("Two sockets facing each other head-on must be accepted."), IsWithinDefaults(A, B))
	CHECK_MESSAGE(TEXT("The gate must not depend on which junction is passed first."), IsWithinDefaults(B, A))
}

N_TEST_CRITICAL(FNWorldAssemblyUtilsTests_ConnectionAngles_RejectsAHatchJoinedToAWallDoor,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::RejectsAHatchJoinedToAWallDoor",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The case the gate was added for. Given enough room a spline loops from a ceiling hatch around to a wall door
	// gently enough to clear every shape limit downstream, so nothing but an orientation test rejects it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	FNCellJunctionDetails Hatch;
	FNCellJunctionDetails WallDoor;
	MakeHatchAndWallDoor(Hatch, WallDoor);

	CHECK_FALSE_MESSAGE(TEXT("A ceiling hatch must not be paired with a wall door."), IsWithinDefaults(Hatch, WallDoor))
	CHECK_FALSE_MESSAGE(TEXT("The rejection must not depend on argument order."), IsWithinDefaults(WallDoor, Hatch))
}

N_TEST_CRITICAL(FNWorldAssemblyUtilsTests_ConnectionAngles_AcceptsARightAngleBend,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::AcceptsARightAngleBend",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The guard on the case above. Two wall openings meeting at a right angle sit at exactly the same 90 degrees of
	// facing as the hatch does, so a gate resting on facing alone would reject this perfectly ordinary corridor bend
	// along with it. Only the elevation difference tells the two apart, and here it is zero.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails A = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	const FNCellJunctionDetails B = MakeJunctionFacing(FVector(500.0, 500.0, 0.0), -FVector::RightVector);

	CHECK_MESSAGE(TEXT("A right-angle bend between two wall openings must be accepted."), IsWithinDefaults(A, B))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_ConnectionAngles_RejectsAPartnerBehindTheSocket,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::RejectsAPartnerBehindTheSocket",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two sockets that face each other perfectly but open away from one another, back to back. Facing is satisfied;
	// what fails is approach, since reaching the partner means leaving the opening and looping right around the cell
	// it belongs to.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails A = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	const FNCellJunctionDetails B = MakeJunctionFacing(FVector(-1000.0, 0.0, 0.0), -FVector::ForwardVector);

	CHECK_FALSE_MESSAGE(TEXT("A partner sitting directly behind the socket must be rejected."), IsWithinDefaults(A, B))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_ConnectionAngles_RejectsSocketsOpeningTheSameWay,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::RejectsSocketsOpeningTheSameWay",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two openings in series, both onto the same direction: the second one's back is what the first one sees. The
	// partner is straight ahead so approach is satisfied, leaving facing — a full 180 — as the rejection.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails A = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	const FNCellJunctionDetails B = MakeJunctionFacing(FVector(500.0, 0.0, 0.0), FVector::ForwardVector);

	CHECK_FALSE_MESSAGE(TEXT("Two sockets opening onto the same direction must be rejected."), IsWithinDefaults(A, B))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_ConnectionAngles_AcceptsAJogBetweenParallelCorridors,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::AcceptsAJogBetweenParallelCorridors",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two opposed openings offset far enough sideways that each sits almost abeam the other — approach lands near 82
	// degrees. This documents where the shipped approach limit is deliberately drawn: at 90, so the partner only has
	// to be in front of the opening rather than in front of *and* squared up to it. Tightening it much would take
	// this ordinary S-bend with it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails A = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	const FNCellJunctionDetails B = MakeJunctionFacing(FVector(200.0, 1500.0, 0.0), -FVector::ForwardVector);

	CHECK_MESSAGE(TEXT("A jog between two parallel corridors must be accepted."), IsWithinDefaults(A, B))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_ConnectionAngles_AcceptsEverythingWhenLimitsAreOpen,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::AcceptsEverythingWhenLimitsAreOpen",
	N_TEST_CONTEXT_ANYWHERE)
{
	// 180 on every limit is the documented opt-out, and it has to be a real one: the pairing the gate exists to
	// reject must come straight back through once the limits are opened.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	FNCellJunctionDetails Hatch;
	FNCellJunctionDetails WallDoor;
	MakeHatchAndWallDoor(Hatch, WallDoor);

	CHECK_MESSAGE(TEXT("Limits of 180 must accept a pairing the defaults reject."),
		FNWorldAssemblyUtils::AreJunctionsWithinConnectionAngles(Hatch, WallDoor, NoLimit, NoLimit, NoLimit))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_ConnectionAngles_SkipsApproachForCoincidentSockets,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::SkipsApproachForCoincidentSockets",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two junctions in the same place have no line between them for an approach angle to be measured against. This
	// is reachable whenever Connect Coincidences is off, so it has to resolve rather than divide by zero — the pair
	// is left for the solver to report degenerate.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails A = MakeJunctionFacing(FVector(300.0, -200.0, 50.0), FVector::ForwardVector);
	const FNCellJunctionDetails B = MakeJunctionFacing(FVector(300.0, -200.0, 50.0), -FVector::ForwardVector);

	CHECK_MESSAGE(TEXT("A coincident opposed pair must clear the gate rather than trip the approach test."),
		IsWithinDefaults(A, B))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_ConnectionAngles_StricterOverrideNarrowsThePair,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::StricterOverrideNarrowsThePair",
	N_TEST_CONTEXT_ANYWHERE)
{
	// One end opting into a tighter facing limit has to bind the whole pairing, from either side of it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	// 45 degrees of facing: comfortably inside the default, comfortably outside the override below.
	const FNCellJunctionDetails A = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	FNCellJunctionDetails B = MakeJunctionFacing(FVector(1000.0, 0.0, 0.0), FVector(-1.0, -1.0, 0.0));

	CHECK_MESSAGE(TEXT("The pair must be accepted before any override is applied."), IsWithinDefaults(A, B))

	B.ConnectionConstraints.bOverrideAngleLimits = true;
	B.ConnectionConstraints.MaximumFacingAngle = 30.f;

	CHECK_FALSE_MESSAGE(TEXT("A stricter facing override on one end must reject the pair."), IsWithinDefaults(A, B))
	CHECK_FALSE_MESSAGE(TEXT("The override must bind from either argument order."), IsWithinDefaults(B, A))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_ConnectionAngles_PermissiveOverrideDoesNotWidenThePair,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::PermissiveOverrideDoesNotWidenThePair",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The other half of the veto rule. Both ends are consulted and the stricter wins, so opting in with a wide limit
	// buys nothing against a partner still on a narrow one — an override can only ever narrow.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	FNCellJunctionDetails Hatch;
	FNCellJunctionDetails WallDoor;
	MakeHatchAndWallDoor(Hatch, WallDoor);

	Hatch.ConnectionConstraints.bOverrideAngleLimits = true;
	Hatch.ConnectionConstraints.MaximumFacingAngle = NoLimit;
	Hatch.ConnectionConstraints.MaximumApproachAngle = NoLimit;
	Hatch.ConnectionConstraints.MaximumElevationDifference = NoLimit;

	CHECK_FALSE_MESSAGE(TEXT("A permissive override on one end must not loosen the limit its partner still carries."),
		IsWithinDefaults(Hatch, WallDoor))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_ConnectionAngles_OverridesOnBothEndsOpenTheGate,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::OverridesOnBothEndsOpenTheGate",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Following from the rule above: exempting a pairing takes an override on *both* ends, since a single default
	// left standing is enough to hold the limit. This is the authored path to a deliberate hatch-to-wall connector.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	FNCellJunctionDetails Hatch;
	FNCellJunctionDetails WallDoor;
	MakeHatchAndWallDoor(Hatch, WallDoor);

	FNCellJunctionConnectionConstraints Open;
	Open.bOverrideAngleLimits = true;
	Open.MaximumFacingAngle = NoLimit;
	Open.MaximumApproachAngle = NoLimit;
	Open.MaximumElevationDifference = NoLimit;

	Hatch.ConnectionConstraints = Open;
	WallDoor.ConnectionConstraints = Open;

	CHECK_MESSAGE(TEXT("Overrides on both ends must open the gate."), IsWithinDefaults(Hatch, WallDoor))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_ConnectionAngles_AcceptsARampMeetingACorridor,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::ConnectionAngles::AcceptsARampMeetingACorridor",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The elevation limit is a difference rather than a ban on sloped openings: a ramp mouth pitched 30 degrees up
	// still meets a level corridor inside the default, which is what keeps the gate from flattening a layout.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNConnectionAnglesHarness;

	const FNCellJunctionDetails Corridor = MakeJunctionFacing(FVector::ZeroVector, FVector::ForwardVector);
	const FNCellJunctionDetails Ramp = MakeJunctionFacing(FVector(1000.0, 0.0, 250.0),
		FVector(-FMath::Cos(FMath::DegreesToRadians(30.0)), 0.0, -FMath::Sin(FMath::DegreesToRadians(30.0))));

	CHECK_MESSAGE(TEXT("A ramp mouth meeting a level corridor must be accepted."), IsWithinDefaults(Corridor, Ramp))
}

#endif //WITH_TESTS
