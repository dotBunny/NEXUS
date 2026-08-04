// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyUtils.h"
#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Cell/NCellJunctionDetails.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness
{
	/** World size of one socket grid unit, matching the UNWorldAssemblySettings default. */
	static const FVector2D UnitSize = FVector2D(100.0, 100.0);

	/** Matches the constant FNConnectJunctionsTask routes through the predicate. */
	constexpr float Tolerance = 1.f;

	/** @return A world-space junction at Location facing Rotation, with the default 2x4 socket unless overridden. */
	static FNCellJunctionDetails MakeJunction(const FVector& Location, const FRotator& Rotation,
		const FIntVector2& SocketSize = FIntVector2(2, 4))
	{
		FNCellJunctionDetails Details;
		Details.WorldLocation = Location;
		Details.WorldRotation = Rotation;
		Details.SocketSize = SocketSize;
		return Details;
	}

	/**
	 * The world rotation a junction ends up with once the builder mates its cell against Source.
	 *
	 * Composed from FNVirtualOrganContext::GetRequiredJunctionRotation — the same helper the placement math in
	 * ProcessCellNode goes through — so a fixture built here is the pose the builder actually produces rather than
	 * an independent guess at it. The required rotation is what the *cell* takes on, so the junction's own authored
	 * rotation is applied on top to land back in world space.
	 * @param Source The junction being mated against.
	 * @param AuthoredLocalRotation The mating junction's authored rotation within its cell.
	 * @return The mating junction's world rotation.
	 */
	static FRotator MateAgainst(const FNCellJunctionDetails& Source, const FRotator& AuthoredLocalRotation)
	{
		const FRotator CellRotation = FNVirtualOrganContext::GetRequiredJunctionRotation(
			Source.WorldRotation.Quaternion(), AuthoredLocalRotation);

		return (CellRotation.Quaternion() * AuthoredLocalRotation.Quaternion()).Rotator();
	}

	/** @return AreJunctionsInverseCoincident under the harness unit size and tolerance. */
	static bool IsInverseCoincident(const FNCellJunctionDetails& A, const FNCellJunctionDetails& B)
	{
		return FNWorldAssemblyUtils::AreJunctionsInverseCoincident(A, B, UnitSize, Tolerance);
	}
}

//
// AreJunctionsInverseCoincident — the gate FNConnectJunctionsTask mates two flush junctions on.
//

N_TEST_CRITICAL(FNWorldAssemblyUtilsTests_InverseCoincident_AcceptsABuilderMating,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::AcceptsABuilderMating",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The load-bearing case, and the reason the predicate exists: a pair posed exactly as the builder poses a mating
	// it makes itself must be recognised as one. Deliberately awkward rotations on both the source and the mating
	// junction's authored pose, so this cannot pass by accident on an axis-aligned fixture.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FVector Location(1200.0, -450.0, 325.0);
	const FNCellJunctionDetails Source = MakeJunction(Location, FRotator(35.f, 110.f, -25.f));
	const FNCellJunctionDetails Mated = MakeJunction(Location, MateAgainst(Source, FRotator(12.f, -40.f, 8.f)));

	CHECK_MESSAGE(TEXT("A pair posed as the builder mates them must read as inverse coincident."),
		IsInverseCoincident(Source, Mated))
	CHECK_MESSAGE(TEXT("The test must not depend on which junction is passed first."),
		IsInverseCoincident(Mated, Source))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_InverseCoincident_AcceptsAVerticalMating,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::AcceptsAVerticalMating",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A junction opening straight down mates with one opening straight up. Nothing in the predicate may assume a
	// turn plane or a yaw-only pose — and this is the pose that would break a rotator-based shortcut, since a
	// pitch of 90 is gimbal lock.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FVector Location(0.0, 0.0, 800.0);
	const FNCellJunctionDetails Source = MakeJunction(Location, FRotator(90.f, 0.f, 0.f));
	const FNCellJunctionDetails Mated = MakeJunction(Location, MateAgainst(Source, FRotator::ZeroRotator));

	CHECK_MESSAGE(TEXT("A vertical mating must read as inverse coincident."), IsInverseCoincident(Source, Mated))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_InverseCoincident_RejectsSameFacing,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::RejectsSameFacing",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two junctions in the same place opening onto the *same* direction are two cells stacked back to back, not a
	// mating. Their socket rectangles are identical, so only the facing test can tell them apart — which is why it
	// is not redundant with the corner comparison.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FVector Location(500.0, 500.0, 0.0);
	const FRotator Rotation(0.f, 45.f, 0.f);

	CHECK_FALSE_MESSAGE(TEXT("Two coincident junctions facing the same way must not be mated."),
		IsInverseCoincident(MakeJunction(Location, Rotation), MakeJunction(Location, Rotation)))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_InverseCoincident_RejectsSeparatedSockets,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::RejectsSeparatedSockets",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Correctly opposed but not flush. These are the connector pass's business, not this one's: mating them would
	// leave a gap no geometry closes.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FNCellJunctionDetails Source = MakeJunction(FVector::ZeroVector, FRotator(0.f, 90.f, 0.f));
	const FNCellJunctionDetails Separated = MakeJunction(FVector(0.0, 25.0, 0.0), MateAgainst(Source, FRotator::ZeroRotator));

	CHECK_FALSE_MESSAGE(TEXT("Junctions 25 units apart must not be mated."), IsInverseCoincident(Source, Separated))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_InverseCoincident_AcceptsWithinTolerance,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::AcceptsWithinTolerance",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Coincidence is measured with slack rather than by exact equality, because a real one arrives through a chain
	// of quaternion compositions and never lands bit-identical.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FNCellJunctionDetails Source = MakeJunction(FVector::ZeroVector, FRotator(0.f, 90.f, 0.f));
	const FNCellJunctionDetails Drifted = MakeJunction(FVector(0.0, 0.5, 0.0), MateAgainst(Source, FRotator::ZeroRotator));

	CHECK_MESSAGE(TEXT("A half-unit drift must still read as coincident."), IsInverseCoincident(Source, Drifted))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_InverseCoincident_RejectsMismatchedSocketSize,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::RejectsMismatchedSocketSize",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Perfectly posed, but the two openings are different sizes — the larger one would be left partly walled off.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FVector Location(0.0, 0.0, 0.0);
	const FNCellJunctionDetails Source = MakeJunction(Location, FRotator(0.f, 90.f, 0.f), FIntVector2(2, 4));
	const FNCellJunctionDetails Smaller = MakeJunction(Location, MateAgainst(Source, FRotator::ZeroRotator), FIntVector2(2, 2));

	CHECK_FALSE_MESSAGE(TEXT("A 2x4 socket must not mate with a 2x2 one."), IsInverseCoincident(Source, Smaller))
}

N_TEST_HIGH(FNWorldAssemblyUtilsTests_InverseCoincident_RejectsRolledRectangularSocket,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::RejectsRolledRectangularSocket",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Same place, opposed facing, same socket size — and still not a mating, because one opening is portrait and the
	// other landscape. This is the case a centers-and-normals test would wave through, and the reason the predicate
	// compares the socket rectangles instead.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FVector Location(0.0, 0.0, 0.0);
	const FNCellJunctionDetails Source = MakeJunction(Location, FRotator(0.f, 90.f, 0.f), FIntVector2(2, 4));

	// Rolled about the mated junction's own forward axis, which is the socket normal — so the facing is untouched
	// and only the rectangle turns.
	const FQuat Mated = MateAgainst(Source, FRotator::ZeroRotator).Quaternion();
	const FNCellJunctionDetails Rolled = MakeJunction(Location,
		(Mated * FQuat(FVector::ForwardVector, UE_HALF_PI)).Rotator(), FIntVector2(2, 4));

	CHECK_FALSE_MESSAGE(TEXT("A rectangular socket rolled 90 degrees against its partner must not be mated."),
		IsInverseCoincident(Source, Rolled))
}

N_TEST_MEDIUM(FNWorldAssemblyUtilsTests_InverseCoincident_AcceptsRolledSquareSocket,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyUtils::InverseCoincident::AcceptsRolledSquareSocket",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The same 90 degree roll on a square socket maps the rectangle onto itself, so the two openings really do
	// coincide and this is a genuine mating. Pairs with RejectsRolledRectangularSocket: together they show the
	// predicate is testing the opening rather than the pose.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNInverseCoincidentHarness;

	const FVector Location(0.0, 0.0, 0.0);
	const FNCellJunctionDetails Source = MakeJunction(Location, FRotator(0.f, 90.f, 0.f), FIntVector2(2, 2));

	const FQuat Mated = MateAgainst(Source, FRotator::ZeroRotator).Quaternion();
	const FNCellJunctionDetails Rolled = MakeJunction(Location,
		(Mated * FQuat(FVector::ForwardVector, UE_HALF_PI)).Rotator(), FIntVector2(2, 2));

	CHECK_MESSAGE(TEXT("A square socket rolled 90 degrees still lines up and must be mated."),
		IsInverseCoincident(Source, Rolled))
}

#endif //WITH_TESTS
