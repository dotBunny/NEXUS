// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Cell/NCellJunctionDetails.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetailsHarness
{
	/** A junction with every IsEqual-compared field set to a distinct non-default value. */
	static FNCellJunctionDetails MakeBaseline()
	{
		FNCellJunctionDetails Junction;
		Junction.Type = ENCellJunctionType::OutOnlySocket;
		Junction.Requirements = ENCellJunctionRequirements::Required;
		Junction.SocketSize = FIntVector2(3, 5);
		Junction.WorldLocation = FVector(1.0, 2.0, 3.0);
		Junction.WorldRotation = FRotator(10.0, 20.0, 30.0);
		Junction.Weighting = 7;
		Junction.InstanceIdentifier = 42;
		Junction.RotationConstraints.bEnforceMatchingRotation = true;
		return Junction;
	}
}

N_TEST_HIGH(FNCellJunctionDetailsTests_IsEqual_IdenticalDetailsAreEqual,
	"NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetails::IsEqual::IdenticalDetailsAreEqual",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Two structurally identical junctions compare equal.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetailsHarness;

	const FNCellJunctionDetails A = MakeBaseline();
	const FNCellJunctionDetails B = MakeBaseline();
	CHECK_MESSAGE(TEXT("Identical junction details must compare equal."), A.IsEqual(B))
}

N_TEST_CRITICAL(FNCellJunctionDetailsTests_IsEqual_EachComparedFieldBreaksEquality,
	"NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetails::IsEqual::EachComparedFieldBreaksEquality",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every field IsEqual claims to compare must, when changed alone, flip the result to not-equal. This guards the
	// author-time drift detection against a field silently dropping out of the comparison.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetailsHarness;

	const FNCellJunctionDetails Baseline = MakeBaseline();

	{
		FNCellJunctionDetails Other = Baseline;
		Other.InstanceIdentifier = 43;
		CHECK_FALSE_MESSAGE(TEXT("A differing InstanceIdentifier must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.Weighting = 8;
		CHECK_FALSE_MESSAGE(TEXT("A differing Weighting must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.Requirements = ENCellJunctionRequirements::AllowEmpty;
		CHECK_FALSE_MESSAGE(TEXT("A differing Requirements must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.Type = ENCellJunctionType::InOnlySocket;
		CHECK_FALSE_MESSAGE(TEXT("A differing Type must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.SocketSize = FIntVector2(9, 9);
		CHECK_FALSE_MESSAGE(TEXT("A differing SocketSize must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.WorldLocation = FVector(9.0, 9.0, 9.0);
		CHECK_FALSE_MESSAGE(TEXT("A differing WorldLocation must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.WorldRotation = FRotator(1.0, 1.0, 1.0);
		CHECK_FALSE_MESSAGE(TEXT("A differing WorldRotation must break equality."), Baseline.IsEqual(Other))
	}
	{
		FNCellJunctionDetails Other = Baseline;
		Other.RotationConstraints.bEnforceMatchingRotation = !Baseline.RotationConstraints.bEnforceMatchingRotation;
		CHECK_FALSE_MESSAGE(TEXT("A differing RotationConstraints must break equality."), Baseline.IsEqual(Other))
	}
}

N_TEST_MEDIUM(FNCellJunctionDetailsTests_IsEqual_DerivedCacheIsIgnored,
	"NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetails::IsEqual::DerivedCacheIsIgnored",
	N_TEST_CONTEXT_ANYWHERE)
{
	// CachedInverseWorldQuat is a derived, non-serialized cache and is deliberately excluded from IsEqual, so a
	// difference there must not register as author-time drift.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetailsHarness;

	const FNCellJunctionDetails Baseline = MakeBaseline();
	FNCellJunctionDetails Other = Baseline;
	Other.CachedInverseWorldQuat = FQuat(FVector::UpVector, PI);

	CHECK_MESSAGE(TEXT("A difference only in the derived quaternion cache must not break equality."), Baseline.IsEqual(Other))
}

N_TEST_MEDIUM(FNCellJunctionDetailsTests_CopyTo_ProducesEqualCopy,
	"NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetails::CopyTo::ProducesEqualCopy",
	N_TEST_CONTEXT_ANYWHERE)
{
	// CopyTo yields a value-equal copy of the source.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetailsHarness;

	const FNCellJunctionDetails Source = MakeBaseline();
	FNCellJunctionDetails Destination;
	Source.CopyTo(Destination);

	CHECK_MESSAGE(TEXT("CopyTo must produce a junction equal to the source."), Source.IsEqual(Destination))
}

#endif //WITH_TESTS
