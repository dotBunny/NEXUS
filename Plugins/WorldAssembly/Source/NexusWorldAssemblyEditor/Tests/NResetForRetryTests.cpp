// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualBoneData.h"
#include "Assembly/Graph/NAssemblyGraph.h"
#include "Assembly/Graph/NAssemblyGraphNodeFactory.h"
#include "Cell/NTissueTagGroups.h"
#include "Collections/NGameplayTagCounter.h"
#include "GameplayTagContainer.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor
	// module; request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static FGameplayTag TagA() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")); }
	static FGameplayTag TagB() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")); }
	static FGameplayTag UniqueTag() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Unique")); }

	static FNAssemblyGraphBoneNode* MakeBoneRoot()
	{
		static FNVirtualBoneData BoneData;
		BoneData.SocketSize = FIntVector2(2, 4);
		return FNAssemblyGraphNodeFactory::CreateBoneNode(&BoneData, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

N_TEST_CRITICAL(FNResetForRetryTests_ResetForRetry_ResetsUsedCounts,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::ResetsUsedCounts",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Every candidate cell's UsedCount must be wound back to zero so the next attempt starts from a clean slate;
	// a stale UsedCount would make a cell look exhausted (or over-placed) on the retry.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 4;

	FNVirtualCellData CellA;
	CellA.UsedCount = 3;
	FNVirtualCellData CellB;
	CellB.UsedCount = 1;
	Context.CellInputData.Add(CellA);
	Context.CellInputData.Add(CellB);

	Context.ResetForRetry();

	CHECK_EQUALS("First cell's UsedCount should be reset to 0.", Context.CellInputData[0].UsedCount, 0)
	CHECK_EQUALS("Second cell's UsedCount should be reset to 0.", Context.CellInputData[1].UsedCount, 0)
}

N_TEST_HIGH(FNResetForRetryTests_ResetForRetry_RestoresContextTagsToBase,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::RestoresContextTagsToBase",
	N_TEST_CONTEXT_ANYWHERE)
{
	// ContextTags drift as cells are placed; a retry must snap them back to the base captured at organ start,
	// dropping anything this attempt added while keeping what was there to begin with.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 4;

	Context.BaseContextTags = FGameplayTagContainer(TagA());

	// Simulate an in-progress attempt that added a second context tag.
	Context.ContextTags = FGameplayTagContainer(TagA());
	Context.ContextTags.AddTag(TagB());

	Context.ResetForRetry();

	CHECK_MESSAGE(TEXT("The base context tag must survive the reset."), Context.ContextTags.HasTagExact(TagA()))
	CHECK_FALSE_MESSAGE(TEXT("A context tag added during the attempt must be dropped on reset."),
		Context.ContextTags.HasTagExact(TagB()))
}

N_TEST_HIGH(FNResetForRetryTests_ResetForRetry_RestoresTagCounterToBase,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::RestoresTagCounterToBase",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The tag counter is restored to its base snapshot: counts mutated during the attempt revert, and tags that
	// only appeared this attempt are gone entirely.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 4;

	Context.BaseTagCounter.Add(TagA(), 1);

	// Simulate an attempt that bumped TagA and introduced a brand-new TagB.
	Context.TagCounter = Context.BaseTagCounter;
	Context.TagCounter.Add(TagA(), 4);
	Context.TagCounter.Add(TagB(), 3);

	Context.ResetForRetry();

	CHECK_EQUALS("TagA's count should revert to the base value of 1.", Context.TagCounter.GetValue(TagA()), 1)
	CHECK_FALSE_MESSAGE(TEXT("A tag introduced during the attempt must not survive the reset."),
		Context.TagCounter.Has(TagB()))
}

N_TEST_HIGH(FNResetForRetryTests_ResetForRetry_ClearsPlacedTagGroups,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::ClearsPlacedTagGroups",
	N_TEST_CONTEXT_ANYWHERE)
{
	// PlacedTagGroups accumulates the tags of cells placed this attempt; a retry must wipe it back to empty so
	// the new attempt does not inherit the previous run's uniqueness/required bookkeeping.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 4;

	Context.PlacedTagGroups.UniqueTags.AppendTags(FGameplayTagContainer(UniqueTag()));
	CHECK_MESSAGE(TEXT("Sanity: the unique tag should be present before the reset."),
		Context.PlacedTagGroups.UniqueTags.HasAnyExact(FGameplayTagContainer(UniqueTag())))

	Context.ResetForRetry();

	CHECK_FALSE_MESSAGE(TEXT("PlacedTagGroups must be empty after a reset."),
		Context.PlacedTagGroups.UniqueTags.HasAnyExact(FGameplayTagContainer(UniqueTag())))
}

N_TEST_HIGH(FNResetForRetryTests_ResetForRetry_ClearsGraph,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::ClearsGraph",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The in-progress graph from the failed attempt must be dropped so the next attempt builds from nothing.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 4;

	Context.CellGraph = MakeUnique<FNAssemblyGraph>(MakeBoneRoot(), FVector::ZeroVector, FBoxSphereBounds(ForceInit), true);
	CHECK_MESSAGE(TEXT("Sanity: a graph should be present before the reset."), Context.CellGraph != nullptr)

	Context.ResetForRetry();

	CHECK_MESSAGE(TEXT("The graph must be released after a reset."), Context.CellGraph == nullptr)
}

N_TEST_CRITICAL(FNResetForRetryTests_ResetForRetry_RetryBudgetExhausts,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::RetryBudgetExhausts",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With MaximumRetryCount = 2 the context may reset twice (both return true); the third reset overshoots the
	// budget and must return false so the builder gives up on the organ.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 2;

	CHECK_MESSAGE(TEXT("First reset should be allowed."), Context.ResetForRetry())
	CHECK_MESSAGE(TEXT("Second reset should be allowed."), Context.ResetForRetry())
	CHECK_FALSE_MESSAGE(TEXT("Third reset should exhaust the budget and be refused."), Context.ResetForRetry())
}

N_TEST_MEDIUM(FNResetForRetryTests_ResetForRetry_ZeroRetriesFailsImmediately,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::ResetForRetry::ZeroRetriesFailsImmediately",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A budget of zero means no retries are permitted: the very first reset must already report failure.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNResetForRetryHarness;

	FNVirtualOrganContext Context(1234ull, TEXT("ResetForRetryTest"));
	Context.MaximumRetryCount = 0;

	CHECK_FALSE_MESSAGE(TEXT("With a zero retry budget the first reset must be refused."), Context.ResetForRetry())
}

#endif //WITH_TESTS
