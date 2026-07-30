// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRotationConstraints.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness
{
	// FRotator components are (Pitch, Yaw, Roll); these helpers build a constraint enforcing one interval so each test
	// can pick a window per axis.
	static FNRotationConstraints MatchingWindow(const FRotator& Min, const FRotator& Max)
	{
		FNRotationConstraints Constraints;
		Constraints.bEnforceMatchingRotation = true;
		Constraints.MinimumMatchingRotation = Min;
		Constraints.MaximumMatchingRotation = Max;
		return Constraints;
	}

	static FNRotationConstraints DifferenceWindow(const FRotator& Min, const FRotator& Max)
	{
		FNRotationConstraints Constraints;
		Constraints.bEnforceDifferenceRotation = true;
		Constraints.MinimumDifferenceRotation = Min;
		Constraints.MaximumDifferenceRotation = Max;
		return Constraints;
	}
}

N_TEST_HIGH(FNRotationConstraintsTests_Matching_DisabledAllowsAnything,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::DisabledAllowsAnything",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With enforcement off, the matching test short-circuits to allowed regardless of how far out of any window the
	// rotation is.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	FNRotationConstraints Constraints = MatchingWindow(FRotator(-10.f, -45.f, -10.f), FRotator(10.f, 45.f, 10.f));
	Constraints.bEnforceMatchingRotation = false;

	CHECK_MESSAGE(TEXT("A disabled matching constraint must allow any rotation."),
		Constraints.IsMatchingRotationAllowed(FRotator(80.f, 170.f, 80.f)))
}

N_TEST_HIGH(FNRotationConstraintsTests_Matching_InsideWindowAllowed,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::InsideWindowAllowed",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A rotation inside the window on every axis is allowed.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = MatchingWindow(FRotator(-10.f, -45.f, -10.f), FRotator(10.f, 45.f, 10.f));

	CHECK_MESSAGE(TEXT("A rotation inside the window on all axes must be allowed."),
		Constraints.IsMatchingRotationAllowed(FRotator(0.f, 0.f, 0.f)))
}

N_TEST_HIGH(FNRotationConstraintsTests_Matching_OutsideWindowRejected,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::OutsideWindowRejected",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A rotation outside the window (here on yaw) is rejected.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = MatchingWindow(FRotator(-10.f, -45.f, -10.f), FRotator(10.f, 45.f, 10.f));

	CHECK_FALSE_MESSAGE(TEXT("A yaw of 90 outside the [-45,45] window must be rejected."),
		Constraints.IsMatchingRotationAllowed(FRotator(0.f, 90.f, 0.f)))
}

N_TEST_CRITICAL(FNRotationConstraintsTests_Matching_BoundaryIsInclusive,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::BoundaryIsInclusive",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The window bounds are inclusive (the comparison adds/subtracts a small epsilon), so a rotation sitting exactly on
	// the min or max edge is allowed.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = MatchingWindow(FRotator(-10.f, -45.f, -10.f), FRotator(10.f, 45.f, 10.f));

	CHECK_MESSAGE(TEXT("A yaw exactly on the maximum bound must be allowed."),
		Constraints.IsMatchingRotationAllowed(FRotator(0.f, 45.f, 0.f)))
	CHECK_MESSAGE(TEXT("A yaw exactly on the minimum bound must be allowed."),
		Constraints.IsMatchingRotationAllowed(FRotator(0.f, -45.f, 0.f)))
}

N_TEST_CRITICAL(FNRotationConstraintsTests_Matching_AnySingleAxisOutOfRangeRejects,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::AnySingleAxisOutOfRangeRejects",
	N_TEST_CONTEXT_ANYWHERE)
{
	// All three axes must pass; a rotation inside on yaw and roll but outside on pitch is still rejected.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = MatchingWindow(FRotator(-10.f, -45.f, -10.f), FRotator(10.f, 45.f, 10.f));

	CHECK_FALSE_MESSAGE(TEXT("A pitch of 80 outside its window must reject even when yaw and roll are inside."),
		Constraints.IsMatchingRotationAllowed(FRotator(80.f, 0.f, 0.f)))
}

N_TEST_HIGH(FNRotationConstraintsTests_Matching_NormalizesBeforeComparing,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::NormalizesBeforeComparing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The FRotator overload normalizes each axis before comparing, so an out-of-range raw yaw that wraps into the
	// window passes, and one that wraps out of it fails. Pitch/roll windows are widened so only yaw is exercised.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = MatchingWindow(FRotator(-180.f, -45.f, -180.f), FRotator(180.f, 45.f, 180.f));

	CHECK_MESSAGE(TEXT("A raw yaw of 370 normalizes to 10 and must be allowed."),
		Constraints.IsMatchingRotationAllowed(FRotator(0.f, 370.f, 0.f)))
	CHECK_FALSE_MESSAGE(TEXT("A raw yaw of 190 normalizes to -170 and must be rejected."),
		Constraints.IsMatchingRotationAllowed(FRotator(0.f, 190.f, 0.f)))
}

N_TEST_MEDIUM(FNRotationConstraintsTests_Matching_PreNormalizedOverload,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Matching::PreNormalizedOverload",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The (roll, pitch, yaw) overload skips normalization and compares the supplied values directly.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = MatchingWindow(FRotator(-180.f, -45.f, -180.f), FRotator(180.f, 45.f, 180.f));

	CHECK_MESSAGE(TEXT("A pre-normalized yaw of 10 must be allowed."),
		Constraints.IsMatchingRotationAllowed(0.f, 0.f, 10.f))
	CHECK_FALSE_MESSAGE(TEXT("A pre-normalized yaw of 90 must be rejected."),
		Constraints.IsMatchingRotationAllowed(0.f, 0.f, 90.f))
}

N_TEST_HIGH(FNRotationConstraintsTests_Difference_DisabledByDefaultAllowsAnything,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Difference::DisabledByDefaultAllowsAnything",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The difference interval is opt-in (bEnforceDifferenceRotation defaults false), so by default any delta passes.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints; // defaults: difference enforcement off

	CHECK_MESSAGE(TEXT("With difference enforcement off, any delta rotation must be allowed."),
		Constraints.IsDifferenceRotationAllowed(FRotator(80.f, 170.f, 80.f)))
}

N_TEST_HIGH(FNRotationConstraintsTests_Difference_EnforcedWindowGates,
	"NEXUS::UnitTests::NCore::FNRotationConstraints::Difference::EnforcedWindowGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When enabled, the difference interval gates independently of the matching interval.
	using namespace NEXUS::UnitTests::NCore::FNRotationConstraintsHarness;

	const FNRotationConstraints Constraints = DifferenceWindow(FRotator(-180.f, -45.f, -180.f), FRotator(180.f, 45.f, 180.f));

	CHECK_MESSAGE(TEXT("A delta yaw of 30 inside the difference window must be allowed."),
		Constraints.IsDifferenceRotationAllowed(FRotator(0.f, 30.f, 0.f)))
	CHECK_FALSE_MESSAGE(TEXT("A delta yaw of 90 outside the difference window must be rejected."),
		Constraints.IsDifferenceRotationAllowed(FRotator(0.f, 90.f, 0.f)))
}

#endif //WITH_TESTS
