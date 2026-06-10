// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Collections/NGameplayTagCounterConstraint.h"
#include "Collections/NGameplayTagCounter.h"
#include "Types/NComparisonResult.h"
#include "GameplayTagsManager.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness
{
	// Reuse already-registered tags as distinct identities (resolved by name); the constraint only needs a tag to key
	// against a counter. These built-in tags are registered by the always-loaded NexusWorldAssembly runtime module.
	static FGameplayTag PresentTag() { return UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.WorldAssembly.Counter.Alpha"))); }
	static FGameplayTag AbsentTag() { return UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.WorldAssembly.Counter.Beta"))); }

	/** A counter holding a single tag at the given count. */
	static FNGameplayTagCounter CounterWith(const FGameplayTag& Tag, const int32 Count)
	{
		FNGameplayTagCounter Counter;
		Counter.Add(Tag, Count);
		return Counter;
	}

	/** A single constraint predicate. */
	static FNGameplayTagCounterConstraint MakeConstraint(const FGameplayTag& Tag, const ENComparisonResult Comparison, const int32 Value)
	{
		FNGameplayTagCounterConstraint Constraint;
		Constraint.Tag = Tag;
		Constraint.Comparison = Comparison;
		Constraint.Value = Value;
		return Constraint;
	}
}

N_TEST_CRITICAL(FNGameplayTagCounterConstraintTests_DoesPassComparison_AbsentTagAlwaysFails,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::AbsentTagAlwaysFails",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A constraint on a tag the counter has never seen fails for every comparison — DoesPassComparison short-circuits
	// on Has() and does NOT treat an absent tag as a count of zero. This even includes NotEqual, which would otherwise
	// pass against any non-matching value.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;

	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_FALSE_MESSAGE(TEXT("Equal on an absent tag must fail."),
		MakeConstraint(AbsentTag(), ENComparisonResult::Equal, 0).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("NotEqual on an absent tag must still fail (absence is not zero)."),
		MakeConstraint(AbsentTag(), ENComparisonResult::NotEqual, 999).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("GreaterThanOrEqual on an absent tag must fail."),
		MakeConstraint(AbsentTag(), ENComparisonResult::GreaterThanOrEqual, 0).DoesPassComparison(Counter))
}

N_TEST_HIGH(FNGameplayTagCounterConstraintTests_DoesPassComparison_Equal,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::Equal",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;
	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_MESSAGE(TEXT("Equal passes when the count matches."),
		MakeConstraint(PresentTag(), ENComparisonResult::Equal, 5).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("Equal fails when the count differs."),
		MakeConstraint(PresentTag(), ENComparisonResult::Equal, 4).DoesPassComparison(Counter))
}

N_TEST_HIGH(FNGameplayTagCounterConstraintTests_DoesPassComparison_NotEqual,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::NotEqual",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;
	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_MESSAGE(TEXT("NotEqual passes when the count differs."),
		MakeConstraint(PresentTag(), ENComparisonResult::NotEqual, 4).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("NotEqual fails when the count matches."),
		MakeConstraint(PresentTag(), ENComparisonResult::NotEqual, 5).DoesPassComparison(Counter))
}

N_TEST_HIGH(FNGameplayTagCounterConstraintTests_DoesPassComparison_GreaterThan,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::GreaterThan",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;
	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_MESSAGE(TEXT("GreaterThan passes when strictly greater."),
		MakeConstraint(PresentTag(), ENComparisonResult::GreaterThan, 4).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("GreaterThan fails on equality (not strictly greater)."),
		MakeConstraint(PresentTag(), ENComparisonResult::GreaterThan, 5).DoesPassComparison(Counter))
}

N_TEST_HIGH(FNGameplayTagCounterConstraintTests_DoesPassComparison_GreaterThanOrEqual,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::GreaterThanOrEqual",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;
	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_MESSAGE(TEXT("GreaterThanOrEqual passes on equality."),
		MakeConstraint(PresentTag(), ENComparisonResult::GreaterThanOrEqual, 5).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("GreaterThanOrEqual fails when below the value."),
		MakeConstraint(PresentTag(), ENComparisonResult::GreaterThanOrEqual, 6).DoesPassComparison(Counter))
}

N_TEST_HIGH(FNGameplayTagCounterConstraintTests_DoesPassComparison_LessThan,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::LessThan",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;
	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_MESSAGE(TEXT("LessThan passes when strictly less."),
		MakeConstraint(PresentTag(), ENComparisonResult::LessThan, 6).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("LessThan fails on equality (not strictly less)."),
		MakeConstraint(PresentTag(), ENComparisonResult::LessThan, 5).DoesPassComparison(Counter))
}

N_TEST_HIGH(FNGameplayTagCounterConstraintTests_DoesPassComparison_LessThanOrEqual,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraint::DoesPassComparison::LessThanOrEqual",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterConstraintHarness;
	const FNGameplayTagCounter Counter = CounterWith(PresentTag(), 5);

	CHECK_MESSAGE(TEXT("LessThanOrEqual passes on equality."),
		MakeConstraint(PresentTag(), ENComparisonResult::LessThanOrEqual, 5).DoesPassComparison(Counter))
	CHECK_FALSE_MESSAGE(TEXT("LessThanOrEqual fails when above the value."),
		MakeConstraint(PresentTag(), ENComparisonResult::LessThanOrEqual, 4).DoesPassComparison(Counter))
}

#endif //WITH_TESTS
