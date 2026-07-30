// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Cell/NCellJunctionComponent.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Engine/World.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"
#include "Types/NRawMesh.h"
#include "Types/NRawMeshUtils.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentHullPenetrationHarness
{
	/** Half-extent of the hull a junction's socket corners are measured against. Comfortably larger than the socket. */
	constexpr double LargeHullHalfExtent = 200.0;

	/** Half-extent small enough that every socket corner falls outside the hull, so the sweep reports no penetration. */
	constexpr double SmallHullHalfExtent = 10.0;

	/** Socket sizing passed to the penetration query; explicit so the corner layout does not depend on project settings. */
	static const FVector2D SocketSize = FVector2D(50.0, 50.0);

	/** @return A convex box hull spanning +/-HalfExtent on every axis, in the same space as the junction's world corners. */
	static FNRawMesh MakeHull(const double HalfExtent)
	{
		return FNRawMeshUtils::MakeBoxHull(FBox(FVector(-HalfExtent), FVector(HalfExtent)));
	}

	/**
	 * @return An unregistered junction on a freshly spawned actor, positioned at Location.
	 * @note Deliberately left unregistered — OnRegister expects a cell root in the level and would log for its absence.
	 *       The transform is set through the direct setter plus an explicit UpdateComponentToWorld because the usual
	 *       move path is a no-op on an unregistered component.
	 */
	static UNCellJunctionComponent* MakeJunction(UWorld* World, const FVector& Location)
	{
		ANDebugActor* Owner = World->SpawnActor<ANDebugActor>();
		UNCellJunctionComponent* Junction = NewObject<UNCellJunctionComponent>(Owner);
		Junction->SetRelativeLocation_Direct(Location);
		Junction->UpdateComponentToWorld();
		return Junction;
	}

	/** @return The memoized maximum penetration of Junction's socket corners into Hull. */
	static float Penetration(const UNCellJunctionComponent* Junction, const FNRawMesh& Hull)
	{
		float MaximumDepth = 0.f;
		double LowestCornerZ = 0.0;
		Junction->GetCachedHullPenetration(Hull, SocketSize, MaximumDepth, LowestCornerZ);
		return MaximumDepth;
	}
}

N_TEST_HIGH(UNCellJunctionComponentTests_ComputeMaximumHullPenetration_DeepestCornerWins,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::ComputeMaximumHullPenetration::DeepestCornerWins",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The sweep reports the deepest corner, not the first or the last, and ignores corners that miss the hull entirely.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentHullPenetrationHarness;

	const FNRawMesh Hull = MakeHull(LargeHullHalfExtent);

	// Depth into a convex hull is the distance to the nearest face plane: the centre sits a full half-extent from
	// every face, the near-surface point only 10, and the outside point is not measured at all.
	const TArray<FVector> Corners = {
		FVector(0.0, 0.0, LargeHullHalfExtent - 10.0),
		FVector::ZeroVector,
		FVector(0.0, 0.0, LargeHullHalfExtent * 5.0)
	};

	const float Depth = UNCellJunctionComponent::ComputeMaximumHullPenetration(Hull, Corners);
	CHECK_MESSAGE("The deepest corner determines the result regardless of its position in the array.",
		FMath::IsNearlyEqual(Depth, static_cast<float>(LargeHullHalfExtent), 0.01f))
}

N_TEST_HIGH(UNCellJunctionComponentTests_ComputeMaximumHullPenetration_OutsideReportsZero,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::ComputeMaximumHullPenetration::OutsideReportsZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// GetIntersectDepth answers -1 for a point outside the hull; the sweep must collapse that to 0 rather than
	// letting a negative sentinel escape as a depth the caller would compare against its penetration threshold.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentHullPenetrationHarness;

	const FNRawMesh Hull = MakeHull(SmallHullHalfExtent);
	const TArray<FVector> Corners = {
		FVector(1000.0, 0.0, 0.0),
		FVector(0.0, 1000.0, 0.0)
	};

	CHECK_EQUALS("Corners clear of the hull report no penetration.",
		UNCellJunctionComponent::ComputeMaximumHullPenetration(Hull, Corners), 0.f)
}

N_TEST_HIGH(UNCellJunctionComponentTests_GetCachedHullPenetration_RepeatQueryIsStable,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::GetCachedHullPenetration::RepeatQueryIsStable",
	N_TEST_CONTEXT_EDITOR)
{
	// The memo must return its cached answer, not a different one, when nothing about the inputs has changed —
	// this is the case the ed mode hits on every idle viewport redraw.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentHullPenetrationHarness;

		const FNRawMesh Hull = MakeHull(LargeHullHalfExtent);
		const UNCellJunctionComponent* Junction = MakeJunction(World, FVector::ZeroVector);

		const float First = Penetration(Junction, Hull);
		const float Second = Penetration(Junction, Hull);

		CHECK_MESSAGE("A junction inside the hull reports a positive penetration.", First > 0.f)
		CHECK_EQUALS("The cached answer matches the computed one.", Second, First)
	});
}

N_TEST_HIGH(UNCellJunctionComponentTests_GetCachedHullPenetration_InvalidatesOnTransformChange,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::GetCachedHullPenetration::InvalidatesOnTransformChange",
	N_TEST_CONTEXT_EDITOR)
{
	// Moving the junction must re-run the sweep. Verified against a freshly built junction at the destination
	// rather than by inspecting the cache, so the test measures observable behaviour.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentHullPenetrationHarness;

		const FNRawMesh Hull = MakeHull(LargeHullHalfExtent);
		const FVector FarAway = FVector(10000.0, 0.0, 0.0);

		UNCellJunctionComponent* Junction = MakeJunction(World, FVector::ZeroVector);
		const float Inside = Penetration(Junction, Hull);

		Junction->SetRelativeLocation_Direct(FarAway);
		Junction->UpdateComponentToWorld();
		const float AfterMove = Penetration(Junction, Hull);

		const float Expected = Penetration(MakeJunction(World, FarAway), Hull);

		CHECK_MESSAGE("The two positions genuinely differ, so a stale cache would be detectable.", Inside != Expected)
		CHECK_EQUALS("Moving the junction re-runs the sweep at its new position.", AfterMove, Expected)
	});
}

N_TEST_HIGH(UNCellJunctionComponentTests_GetCachedHullPenetration_InvalidatesOnHullChange,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::GetCachedHullPenetration::InvalidatesOnHullChange",
	N_TEST_CONTEXT_EDITOR)
{
	// Editing the cell hull must re-run the sweep even though the junction has not moved. Both hulls carry the same
	// vertex and loop counts, so only a content-sensitive key (rather than a count check) can tell them apart.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentHullPenetrationHarness;

		const FNRawMesh LargeHull = MakeHull(LargeHullHalfExtent);
		const FNRawMesh SmallHull = MakeHull(SmallHullHalfExtent);

		CHECK_EQUALS("Both hulls carry the same vertex count, so the key cannot lean on that alone.",
			SmallHull.Vertices.Num(), LargeHull.Vertices.Num())

		const UNCellJunctionComponent* Junction = MakeJunction(World, FVector::ZeroVector);

		const float AgainstLarge = Penetration(Junction, LargeHull);
		const float AgainstSmall = Penetration(Junction, SmallHull);
		const float Expected = Penetration(MakeJunction(World, FVector::ZeroVector), SmallHull);

		CHECK_MESSAGE("The junction penetrates the large hull.", AgainstLarge > 0.f)
		CHECK_EQUALS("Swapping the hull re-runs the sweep against the new geometry.", AgainstSmall, Expected)
	});
}

#endif //WITH_TESTS
