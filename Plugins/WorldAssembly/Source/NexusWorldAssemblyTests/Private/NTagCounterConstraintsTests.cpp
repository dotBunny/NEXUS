// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "Collections/NGameplayTagCounter.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "GameplayTagContainer.h"
#include "Types/NComparisonResult.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness
{
	// The gate only cares about a tag's identity as a map key, so any two registered tags will do. We reuse the
	// built-in behavior tags (registered in NWorldAssemblyGameplayTags.cpp) purely as two distinct identities.
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static FGameplayTag CounterTagA() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")); }
	static FGameplayTag CounterTagB() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")); }

	/** Builds a counter holding a single tag at the given count. */
	static FNGameplayTagCounter MakeCounter(const FGameplayTag& InTag, const int32 Count)
	{
		FNGameplayTagCounter Counter;
		Counter.Add(InTag, Count);
		return Counter;
	}

	/** Builds a single constraint predicate. */
	static FNGameplayTagCounterConstraint MakeConstraint(const FGameplayTag& InTag, const ENComparisonResult Comparison, const int32 Value)
	{
		FNGameplayTagCounterConstraint Constraint;
		Constraint.Tag = InTag;
		Constraint.Comparison = Comparison;
		Constraint.Value = Value;
		return Constraint;
	}

	/** Builds a candidate cell carrying the supplied constraints. */
	static FNVirtualCellData MakeCell(const TArray<FNGameplayTagCounterConstraint>& Constraints)
	{
		FNVirtualCellData Cell;
		Cell.TagCounterConstraints = Constraints;
		return Cell;
	}
}

N_TEST_MEDIUM(FNTagCounterConstraintsTests_TagCounterConstraints_NoConstraintsNeverGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::TagCounterConstraints::NoConstraintsNeverGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A candidate with no constraints is unconditionally eligible regardless of counter state.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness;

	const FNVirtualCellData Cell = MakeCell({});
	const FNGameplayTagCounter Counter = MakeCounter(CounterTagA(), 3);

	CHECK_FALSE_MESSAGE(TEXT("A candidate with no constraints must never be gated."),
		FNVirtualOrganContext::IsGatedByTagCounterConstraints(Cell, Counter))
}

N_TEST_HIGH(FNTagCounterConstraintsTests_TagCounterConstraints_SatisfiedConstraintNotGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::TagCounterConstraints::SatisfiedConstraintNotGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A single constraint whose comparison holds against the counter must let the candidate through.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness;

	const FNVirtualCellData Cell = MakeCell({ MakeConstraint(CounterTagA(), ENComparisonResult::GreaterThanOrEqual, 1) });
	const FNGameplayTagCounter Counter = MakeCounter(CounterTagA(), 2);

	CHECK_FALSE_MESSAGE(TEXT("A satisfied constraint (count 2 >= 1) must not gate the candidate."),
		FNVirtualOrganContext::IsGatedByTagCounterConstraints(Cell, Counter))
}

N_TEST_HIGH(FNTagCounterConstraintsTests_TagCounterConstraints_FailedComparisonGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::TagCounterConstraints::FailedComparisonGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The tag is present but the comparison fails, so the constraint fails and the candidate is gated out.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness;

	const FNVirtualCellData Cell = MakeCell({ MakeConstraint(CounterTagA(), ENComparisonResult::GreaterThanOrEqual, 2) });
	const FNGameplayTagCounter Counter = MakeCounter(CounterTagA(), 1);

	CHECK_MESSAGE(TEXT("A failed comparison (count 1 >= 2) must gate the candidate."),
		FNVirtualOrganContext::IsGatedByTagCounterConstraints(Cell, Counter))
}

N_TEST_HIGH(FNTagCounterConstraintsTests_TagCounterConstraints_MissingTagGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::TagCounterConstraints::MissingTagGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A constraint on a tag the counter has never seen compares against a count of zero (DoesPassComparison treats an
	// absent tag as zero), and 0 >= 1 fails, so the constraint fails and the candidate is gated out.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness;

	const FNVirtualCellData Cell = MakeCell({ MakeConstraint(CounterTagA(), ENComparisonResult::GreaterThanOrEqual, 1) });
	const FNGameplayTagCounter Counter; // empty — CounterTagA is absent

	CHECK_MESSAGE(TEXT("A constraint on an untracked tag must gate the candidate."),
		FNVirtualOrganContext::IsGatedByTagCounterConstraints(Cell, Counter))
}

N_TEST_HIGH(FNTagCounterConstraintsTests_TagCounterConstraints_AllConstraintsPassNotGated,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::TagCounterConstraints::AllConstraintsPassNotGated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With multiple constraints, the candidate is eligible only when every one passes.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness;

	const FNVirtualCellData Cell = MakeCell({
		MakeConstraint(CounterTagA(), ENComparisonResult::GreaterThanOrEqual, 1),
		MakeConstraint(CounterTagB(), ENComparisonResult::Equal, 5),
	});

	FNGameplayTagCounter Counter;
	Counter.Add(CounterTagA(), 2);
	Counter.Add(CounterTagB(), 5);

	CHECK_FALSE_MESSAGE(TEXT("All constraints passing (A 2 >= 1, B 5 == 5) must not gate the candidate."),
		FNVirtualOrganContext::IsGatedByTagCounterConstraints(Cell, Counter))
}

N_TEST_CRITICAL(FNTagCounterConstraintsTests_TagCounterConstraints_OneFailingConstraintGates,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::TagCounterConstraints::OneFailingConstraintGates",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The core all-must-pass invariant: a single failing constraint gates the candidate even when the others
	// pass. This is the regression guard for the original nested-loop bug where a failing constraint was ignored.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNTagCounterConstraintsHarness;

	const FNVirtualCellData Cell = MakeCell({
		MakeConstraint(CounterTagA(), ENComparisonResult::GreaterThanOrEqual, 1),  // passes
		MakeConstraint(CounterTagB(), ENComparisonResult::GreaterThanOrEqual, 10), // fails
	});

	FNGameplayTagCounter Counter;
	Counter.Add(CounterTagA(), 2);
	Counter.Add(CounterTagB(), 5);

	CHECK_MESSAGE(TEXT("A single failing constraint (B 5 >= 10) must gate the candidate despite A passing."),
		FNVirtualOrganContext::IsGatedByTagCounterConstraints(Cell, Counter))
}

#endif //WITH_TESTS
