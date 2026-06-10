// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Collections/NGameplayTagCounter.h"
#include "Collections/NGameplayTagCounterOperation.h"
#include "GameplayTagsManager.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness
{
	// The counter only uses tags as distinct map-key identities, so we reuse three already-registered tags resolved
	// by name rather than defining new ones (native tags cannot be defined in an editor-type module). These built-in
	// counter tags are registered by the always-loaded NexusWorldAssembly runtime module.
	static FGameplayTag TagX() { return UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.WorldAssembly.Counter.Alpha"))); }
	static FGameplayTag TagY() { return UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.WorldAssembly.Counter.Beta"))); }
	static FGameplayTag TagZ() { return UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.WorldAssembly.Counter.Charlie"))); }

	/** Build a declarative counter operation. */
	static FNGameplayTagCounterOperation MakeOp(const FGameplayTag& Tag, const ENGameplayTagCounterOperationType Type, const int32 Value)
	{
		FNGameplayTagCounterOperation Operation;
		Operation.Tag = Tag;
		Operation.Operation = Type;
		Operation.Value = Value;
		return Operation;
	}
}

N_TEST_CRITICAL(FNGameplayTagCounterTests_Operations_AddThenReverseRestoresOriginal,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::Operations::AddThenReverseRestoresOriginal",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Applying an Add then reversing it returns the tag to its starting count — the round-trip RemoveCellNode relies on.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	FNGameplayTagCounter Counter;
	Counter.Add(TagX(), 5);

	const FNGameplayTagCounterOperation Op = MakeOp(TagX(), ENGameplayTagCounterOperationType::Add, 3);
	Counter.ApplyOperation(Op);
	CHECK_EQUALS("Add of 3 onto 5 should yield 8.", Counter.GetValue(TagX()), 8)

	Counter.ReverseOperation(Op);
	CHECK_EQUALS("Reversing the Add should restore the original 5.", Counter.GetValue(TagX()), 5)
}

N_TEST_CRITICAL(FNGameplayTagCounterTests_Operations_SubtractThenReverseRestoresOriginal,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::Operations::SubtractThenReverseRestoresOriginal",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Applying a Subtract then reversing it restores the original count, provided the value never hit the zero floor.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	FNGameplayTagCounter Counter;
	Counter.Add(TagX(), 5);

	const FNGameplayTagCounterOperation Op = MakeOp(TagX(), ENGameplayTagCounterOperationType::Subtract, 3);
	Counter.ApplyOperation(Op);
	CHECK_EQUALS("Subtract of 3 from 5 should yield 2.", Counter.GetValue(TagX()), 2)

	Counter.ReverseOperation(Op);
	CHECK_EQUALS("Reversing the Subtract should restore the original 5.", Counter.GetValue(TagX()), 5)
}

N_TEST_HIGH(FNGameplayTagCounterTests_Operations_SubtractClampsAtZero,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::Operations::SubtractClampsAtZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The counter is documented to never go below zero; subtracting past zero floors at zero rather than going negative.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	FNGameplayTagCounter Counter;
	Counter.Add(TagX(), 2);
	Counter.Subtract(TagX(), 5);

	CHECK_EQUALS("Subtracting 5 from 2 must clamp at zero, not go negative.", Counter.GetValue(TagX()), 0)
}

N_TEST_MEDIUM(FNGameplayTagCounterTests_Operations_ClampedSubtractReverseIsLossy,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::Operations::ClampedSubtractReverseIsLossy",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Characterizes a real limitation: once a Subtract clamps at zero, the lost magnitude cannot be recovered, so the
	// reverse over-restores (2 -> clamp 0 -> +5 = 5, not 2). The generator avoids this by never driving a counter
	// below zero; this test pins the behavior so any future change to the clamping contract is deliberate.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	FNGameplayTagCounter Counter;
	Counter.Add(TagX(), 2);

	const FNGameplayTagCounterOperation Op = MakeOp(TagX(), ENGameplayTagCounterOperationType::Subtract, 5);
	Counter.ApplyOperation(Op);  // clamps to 0, losing 3
	Counter.ReverseOperation(Op); // adds 5

	CHECK_EQUALS("A clamped Subtract is not losslessly reversible; the reverse over-restores to 5.", Counter.GetValue(TagX()), 5)
}

N_TEST_HIGH(FNGameplayTagCounterTests_GetDifference_IsPerTagSubtraction,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::GetDifference::IsPerTagSubtraction",
	N_TEST_CONTEXT_ANYWHERE)
{
	// GetDifference yields this-minus-other per tag, treating an absent tag as zero on either side, and (unlike
	// Add/Subtract) is not floored — a tag present only in Other comes back negative.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	// Guard the reused-tag dependency: if these registered tags ever disappear they would all resolve to the same
	// invalid identity and the difference math would silently collapse — fail loudly here instead.
	if (!TagX().IsValid() || !TagY().IsValid() || !TagZ().IsValid() ||
		TagX() == TagY() || TagX() == TagZ() || TagY() == TagZ())
	{
		ADD_ERROR("Expected three distinct registered counter tags (NEXUS.WorldAssembly.Counter.*).");
		return;
	}

	FNGameplayTagCounter A;
	A.Add(TagX(), 5);
	A.Add(TagY(), 2);

	FNGameplayTagCounter B;
	B.Add(TagX(), 3);
	B.Add(TagZ(), 4);

	FNGameplayTagCounter Diff = A.GetDifference(B);
	CHECK_EQUALS("Tag present in both is the signed difference (5 - 3).", Diff.GetValue(TagX()), 2)
	CHECK_EQUALS("Tag present only in A passes through unchanged (2).", Diff.GetValue(TagY()), 2)
	CHECK_EQUALS("Tag present only in B comes back negative (0 - 4).", Diff.GetValue(TagZ()), -4)
}

N_TEST_MEDIUM(FNGameplayTagCounterTests_GetDifference_IsAntisymmetric,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::GetDifference::IsAntisymmetric",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A.GetDifference(B) is the per-tag negation of B.GetDifference(A).
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	FNGameplayTagCounter A;
	A.Add(TagX(), 5);

	FNGameplayTagCounter B;
	B.Add(TagX(), 3);

	FNGameplayTagCounter AB = A.GetDifference(B);
	FNGameplayTagCounter BA = B.GetDifference(A);
	CHECK_EQUALS("A - B should be +2.", AB.GetValue(TagX()), 2)
	CHECK_EQUALS("B - A should be the negation, -2.", BA.GetValue(TagX()), -2)
}

N_TEST_HIGH(FNGameplayTagCounterTests_GetDifference_BaseCombineDifferenceRestoresWorking,
	"NEXUS::UnitTests::NCore::FNGameplayTagCounter::GetDifference::BaseCombineDifferenceRestoresWorking",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mirrors the organ-builder hand-off: a Working counter seeded from Base and then mutated hands off only its
	// delta (Working.GetDifference(Base)); combining that delta back into Base must reproduce Working exactly, so the
	// shared world counter is never double-counted.
	using namespace NEXUS::UnitTests::NCore::FNGameplayTagCounterHarness;

	FNGameplayTagCounter Base;
	Base.Add(TagX(), 5);
	Base.Add(TagY(), 1);

	// Working starts as Base, then this organ's cells mutate it.
	FNGameplayTagCounter Working = Base;
	Working.Add(TagX(), 2);      // X: 5 -> 7
	Working.Subtract(TagY(), 1); // Y: 1 -> 0
	Working.Add(TagZ(), 3);      // Z: 0 -> 3 (introduced by this organ)

	// The pass folds only the delta back into the (Base-seeded) world counter.
	FNGameplayTagCounter Combined = Base;
	Combined.Combine(Working.GetDifference(Base));

	CHECK_EQUALS("Combining the delta into Base must reproduce Working for X.", Combined.GetValue(TagX()), Working.GetValue(TagX()))
	CHECK_EQUALS("Combining the delta into Base must reproduce Working for Y.", Combined.GetValue(TagY()), Working.GetValue(TagY()))
	CHECK_EQUALS("Combining the delta into Base must reproduce Working for Z.", Combined.GetValue(TagZ()), Working.GetValue(TagZ()))
}

#endif //WITH_TESTS
