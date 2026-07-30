// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Types/NRotationConstraints.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness
{
	/** Tolerance for comparing rotations as quaternions, sidestepping the +/-180 yaw wrap ambiguity. */
	constexpr double Tolerance = 1.e-3;

	/** A matching-rotation constraint that only permits yaw within [MinYaw, MaxYaw]; pitch and roll stay pinned to 0. */
	static FNRotationConstraints YawWindow(const float MinYaw, const float MaxYaw)
	{
		FNRotationConstraints Constraints;
		Constraints.bEnforceMatchingRotation = true;
		Constraints.MinimumMatchingRotation = FRotator(0.f, MinYaw, 0.f);
		Constraints.MaximumMatchingRotation = FRotator(0.f, MaxYaw, 0.f);
		return Constraints;
	}

	/** Matching constraints with enforcement disabled — allows any required rotation through. */
	static FNRotationConstraints Unconstrained()
	{
		FNRotationConstraints Constraints;
		Constraints.bEnforceMatchingRotation = false;
		return Constraints;
	}
}

//
// GetRequiredJunctionRotation — the quaternion composition Filter.SourceQuat * FQuat(Up, PI) * Junction.Inverse().
//

N_TEST_CRITICAL(FNJunctionRotationTests_JunctionRotation_OpposesSourceFacing,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::OpposesSourceFacing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A source facing +90 yaw, against a junction with no authored rotation, must yield a required rotation that
	// opposes it: +90 plus the 180 flip lands on -90 (270 normalized). This is the core "face the source" rule.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	const FRotator Required = FNVirtualOrganContext::GetRequiredJunctionRotation(
		FRotator(0.f, 90.f, 0.f).Quaternion(), FRotator::ZeroRotator);

	CHECK_MESSAGE(TEXT("A +90 source facing should require a -90 cell rotation to oppose it."),
		Required.Quaternion().Equals(FRotator(0.f, -90.f, 0.f).Quaternion(), Tolerance))
}

N_TEST_HIGH(FNJunctionRotationTests_JunctionRotation_UndoesJunctionLocalRotation,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::UndoesJunctionLocalRotation",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no source facing, a junction authored at +90 yaw must have that local rotation undone: the 180 flip
	// minus the junction's own +90 leaves a required +90 (180 - 90). Proves the Inverse() term subtracts the
	// junction's authored pose rather than adding it.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	const FRotator Required = FNVirtualOrganContext::GetRequiredJunctionRotation(
		FQuat::Identity, FRotator(0.f, 90.f, 0.f));

	CHECK_MESSAGE(TEXT("A +90 junction-local rotation should resolve to a +90 required rotation."),
		Required.Quaternion().Equals(FRotator(0.f, 90.f, 0.f).Quaternion(), Tolerance))
}

//
// IsGatedByJunctionRotation — the cell-and-junction matching-rotation veto over the required rotation.
//

N_TEST_HIGH(FNJunctionRotationTests_JunctionRotation_OpposingSourceNeedsNoRotation,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::OpposingSourceNeedsNoRotation",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A source already facing 180 means the candidate needs no rotation at all to mate (180 + 180 = 360 = 0), so
	// even a tight window centered on zero must allow it on both the cell and the junction side.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	const FQuat Source = FRotator(0.f, 180.f, 0.f).Quaternion();

	CHECK_FALSE_MESSAGE(TEXT("A required rotation of zero must pass a tight zero-centered window."),
		FNVirtualOrganContext::IsGatedByJunctionRotation(Source, FRotator::ZeroRotator, YawWindow(-5.f, 5.f), YawWindow(-5.f, 5.f)))
}

N_TEST_HIGH(FNJunctionRotationTests_JunctionRotation_JunctionConstraintVetoes,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::JunctionConstraintVetoes",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Identity source against an unrotated junction requires a 180 rotation. A permissive cell allows it, but a
	// junction window that excludes 180 must veto, gating the junction out.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	CHECK_MESSAGE(TEXT("A junction window excluding the required 180 rotation must gate the junction out."),
		FNVirtualOrganContext::IsGatedByJunctionRotation(FQuat::Identity, FRotator::ZeroRotator, FNRotationConstraints(), YawWindow(-5.f, 5.f)))
}

N_TEST_HIGH(FNJunctionRotationTests_JunctionRotation_CellConstraintVetoesIndependently,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::CellConstraintVetoesIndependently",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The same 180 required rotation, but now the cell carries the tight window and the junction is permissive.
	// The cell-wide veto must gate the junction out on its own, proving the two constraint sets are OR-combined.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	CHECK_MESSAGE(TEXT("A cell window excluding the required 180 rotation must gate the junction out on its own."),
		FNVirtualOrganContext::IsGatedByJunctionRotation(FQuat::Identity, FRotator::ZeroRotator, YawWindow(-5.f, 5.f), FNRotationConstraints()))
}

N_TEST_MEDIUM(FNJunctionRotationTests_JunctionRotation_DisabledEnforcementAllowsAnything,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::DisabledEnforcementAllowsAnything",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When neither side enforces matching rotation, even the 180 required rotation must pass.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	CHECK_FALSE_MESSAGE(TEXT("With enforcement disabled on both sides no rotation must ever gate the junction."),
		FNVirtualOrganContext::IsGatedByJunctionRotation(FQuat::Identity, FRotator::ZeroRotator, Unconstrained(), Unconstrained()))
}

N_TEST_MEDIUM(FNJunctionRotationTests_JunctionRotation_WindowSignMatchesComputedYaw,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::JunctionRotation::WindowSignMatchesComputedYaw",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Ties the math to the gate: a +90 source resolves to a -90 required rotation, so a window around -90 must
	// allow it while the mirrored window around +90 must reject it — confirming the sign of the required rotation
	// actually reaches the constraint check.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNJunctionRotationHarness;

	const FQuat Source = FRotator(0.f, 90.f, 0.f).Quaternion();

	CHECK_FALSE_MESSAGE(TEXT("A window around -90 must allow the -90 required rotation."),
		FNVirtualOrganContext::IsGatedByJunctionRotation(Source, FRotator::ZeroRotator, FNRotationConstraints(), YawWindow(-95.f, -85.f)))
	CHECK_MESSAGE(TEXT("The mirrored window around +90 must reject the -90 required rotation."),
		FNVirtualOrganContext::IsGatedByJunctionRotation(Source, FRotator::ZeroRotator, FNRotationConstraints(), YawWindow(85.f, 95.f)))
}

#endif //WITH_TESTS
