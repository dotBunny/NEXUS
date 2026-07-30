// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Types/NCardinalDirection.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness
{
	/** Default per-heading half-angle the gate compares against in these tests. */
	constexpr float Tolerance = 0.01f;

	/**
	 * Mirrors the bearing FilterCellInputData feeds the gate: the compass heading from the organ's start point
	 * out to a candidate's world position. FVector::Rotation() derives yaw from atan2(Y, X), so North (+X) = 0
	 * and East (+Y) = 90. Kept in lock-step with the production line so these tests pin that convention.
	 */
	static float BearingFromStart(const FVector& StartPoint, const FVector& WorldPosition)
	{
		return static_cast<float>((WorldPosition - StartPoint).Rotation().Yaw);
	}
}

N_TEST_HIGH(FNDirectionalConstraintTests_IsGatedByDirectionalConstraint_MatchingDirectionNotGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByDirectionalConstraint::MatchingDirectionNotGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A candidate whose bearing sits on the constrained heading survives the gate (returns false == not gated).
	CHECK_FALSE_MESSAGE(TEXT("90 degrees against East should not be gated"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(90.f, ENCardinalDirection::East, NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness::Tolerance));
	CHECK_FALSE_MESSAGE(TEXT("0 degrees against North should not be gated"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(0.f, ENCardinalDirection::North, NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness::Tolerance));
}

N_TEST_HIGH(FNDirectionalConstraintTests_IsGatedByDirectionalConstraint_WrongDirectionGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByDirectionalConstraint::WrongDirectionGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A bearing that does not line up with the constrained heading is gated out (returns true == gated).
	CHECK_MESSAGE(TEXT("0 degrees against East should be gated"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(0.f, ENCardinalDirection::East, NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness::Tolerance));
	CHECK_MESSAGE(TEXT("180 degrees against North should be gated"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(180.f, ENCardinalDirection::North, NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness::Tolerance));
}

N_TEST_HIGH(FNDirectionalConstraintTests_IsGatedByDirectionalConstraint_Tolerance,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByDirectionalConstraint::Tolerance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The collapse passed as Tolerance widens what counts as "on" the heading: 100 degrees is gated against East
	// at a tight tolerance but admitted once the tolerance opens past the 10-degree deviation.
	CHECK_MESSAGE(TEXT("100 degrees against East is gated at a tight tolerance"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(100.f, ENCardinalDirection::East, 0.01f));
	CHECK_FALSE_MESSAGE(TEXT("100 degrees against East is admitted under a 15-degree tolerance"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(100.f, ENCardinalDirection::East, 15.f));
}

N_TEST_HIGH(FNDirectionalConstraintTests_IsGatedByDirectionalConstraint_Wraparound,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::IsGatedByDirectionalConstraint::Wraparound",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A near-360 bearing wraps to North, so it is not gated by a North constraint within a 1-degree tolerance.
	CHECK_FALSE_MESSAGE(TEXT("359.5 degrees against North is not gated within a 1-degree tolerance"),
		FNVirtualOrganContext::IsGatedByDirectionalConstraint(359.5f, ENCardinalDirection::North, 1.f));
}

N_TEST_HIGH(FNDirectionalConstraintTests_StartPointBearing_CardinalAxes,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::StartPointBearing::CardinalAxes",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Pin the convention FilterCellInputData relies on: a candidate offset along +X from the start reads as North,
	// +Y as East, -X as South and -Y as West, so each is admitted only by its own heading.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness;
	const FVector Start(500.0, 500.0, 500.0);

	const float NorthBearing = BearingFromStart(Start, Start + FVector(100.0, 0.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("+X candidate should not be gated by North"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(NorthBearing, ENCardinalDirection::North, Tolerance));
	CHECK_MESSAGE(TEXT("+X candidate should be gated by East"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(NorthBearing, ENCardinalDirection::East, Tolerance));

	const float EastBearing = BearingFromStart(Start, Start + FVector(0.0, 100.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("+Y candidate should not be gated by East"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(EastBearing, ENCardinalDirection::East, Tolerance));

	const float SouthBearing = BearingFromStart(Start, Start + FVector(-100.0, 0.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("-X candidate should not be gated by South"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(SouthBearing, ENCardinalDirection::South, Tolerance));

	const float WestBearing = BearingFromStart(Start, Start + FVector(0.0, -100.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("-Y candidate should not be gated by West"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(WestBearing, ENCardinalDirection::West, Tolerance));
}

N_TEST_HIGH(FNDirectionalConstraintTests_StartPointBearing_DiagonalAndVertical,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::StartPointBearing::DiagonalAndVertical",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An equal +X/+Y offset reads as NorthEast (45), and a pure vertical (Z) offset leaves the heading unchanged
	// because yaw ignores Z — a candidate directly above the start still reads as North.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNDirectionalConstraintHarness;
	const FVector Start(0.0, 0.0, 0.0);

	const float DiagonalBearing = BearingFromStart(Start, FVector(100.0, 100.0, 0.0));
	CHECK_FALSE_MESSAGE(TEXT("+X/+Y candidate should not be gated by NorthEast"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(DiagonalBearing, ENCardinalDirection::NorthEast, Tolerance));

	const float NorthWithHeight = BearingFromStart(Start, FVector(100.0, 0.0, 250.0));
	CHECK_FALSE_MESSAGE(TEXT("+X candidate with a Z offset still reads as North"), FNVirtualOrganContext::IsGatedByDirectionalConstraint(NorthWithHeight, ENCardinalDirection::North, Tolerance));
}

N_TEST_HIGH(FNDirectionalConstraintTests_ResolveDirectionTargetPosition_StartBoneUsesBone,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResolveDirectionTargetPosition::StartBoneUsesBone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// StartBone always resolves to the start bone, regardless of the volume bounds.
	const FVector StartBone(10.0, 20.0, 30.0);
	const FBoxSphereBounds Bounds(FVector(1000.0, 2000.0, 3000.0), FVector(50.0, 50.0, 50.0), 50.0);

	const FVector Resolved = FNVirtualOrganContext::ResolveDirectionTargetPosition(
		ENOrganDirectionConstraintMode::StartBone, false, Bounds, StartBone);

	CHECK_MESSAGE(TEXT("StartBone mode must resolve to the start bone position."), Resolved.Equals(StartBone));
}

N_TEST_HIGH(FNDirectionalConstraintTests_ResolveDirectionTargetPosition_OrganCenterUsesBoundsOrigin,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResolveDirectionTargetPosition::OrganCenterUsesBoundsOrigin",
	N_TEST_CONTEXT_ANYWHERE)
{
	// OrganCenter on a bounded organ resolves to the volume's geometric center (Bounds.Origin), not the start bone.
	const FVector StartBone(10.0, 20.0, 30.0);
	const FBoxSphereBounds Bounds(FVector(1000.0, 2000.0, 3000.0), FVector(50.0, 50.0, 50.0), 50.0);

	const FVector Resolved = FNVirtualOrganContext::ResolveDirectionTargetPosition(
		ENOrganDirectionConstraintMode::OrganCenter, false, Bounds, StartBone);

	CHECK_MESSAGE(TEXT("OrganCenter mode on a bounded organ must resolve to Bounds.Origin."), Resolved.Equals(Bounds.Origin));
}

N_TEST_HIGH(FNDirectionalConstraintTests_ResolveDirectionTargetPosition_OrganCenterUnboundFallsBackToBone,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResolveDirectionTargetPosition::OrganCenterUnboundFallsBackToBone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// An unbound organ has a degenerate bounds whose Origin is meaningless, so OrganCenter falls back to the start bone.
	const FVector StartBone(10.0, 20.0, 30.0);
	const FBoxSphereBounds Bounds(FVector(1000.0, 2000.0, 3000.0), FVector(50.0, 50.0, 50.0), 50.0);

	const FVector Resolved = FNVirtualOrganContext::ResolveDirectionTargetPosition(
		ENOrganDirectionConstraintMode::OrganCenter, true, Bounds, StartBone);

	CHECK_MESSAGE(TEXT("OrganCenter mode on an unbound organ must fall back to the start bone."), Resolved.Equals(StartBone));
}

N_TEST_HIGH(FNDirectionalConstraintTests_ResolveDirectionTargetPosition_DynamicCentroidSeedsToBone,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResolveDirectionTargetPosition::DynamicCentroidSeedsToBone",
	N_TEST_CONTEXT_ANYWHERE)
{
	// DynamicCentroid resolves to the start bone here (its pre-placement seed); the live centroid is applied per
	// filter pass in FilterCellInputData, not by this static resolver.
	const FVector StartBone(10.0, 20.0, 30.0);
	const FBoxSphereBounds Bounds(FVector(1000.0, 2000.0, 3000.0), FVector(50.0, 50.0, 50.0), 50.0);

	const FVector Resolved = FNVirtualOrganContext::ResolveDirectionTargetPosition(
		ENOrganDirectionConstraintMode::DynamicCentroid, false, Bounds, StartBone);

	CHECK_MESSAGE(TEXT("DynamicCentroid mode must seed to the start bone in the static resolver."), Resolved.Equals(StartBone));
}

#endif //WITH_TESTS
