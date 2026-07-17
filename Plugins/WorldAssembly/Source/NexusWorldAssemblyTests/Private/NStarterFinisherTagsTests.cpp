// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Assembly/Contexts/NVirtualOrganContext.h"
#include "Assembly/Data/NVirtualCellData.h"
#include "GameplayTagContainer.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness
{
	// The built-in tag symbols are declared without an export macro, so they can't be linked from the editor
	// module; request them by their registered name instead (matches the strings in NWorldAssemblyGameplayTags.cpp).
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static FGameplayTag Starter() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")); }
	static FGameplayTag StarterOnly() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.StarterOnly")); }
	static FGameplayTag NotStarter() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.NotStarter")); }
	static FGameplayTag Finisher() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")); }
	static FGameplayTag FinisherOnly() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.FinisherOnly")); }
	static FGameplayTag NotFinisher() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.NotFinisher")); }

	/** An empty tag set standing in for a cell that carries no starter/finisher markers. */
	static FGameplayTagContainer Untagged() { return FGameplayTagContainer(); }

	/** A summary whose starter flags advertise that the pool contains start-specific content. */
	static FNVirtualCellDataSummary SummaryWithStarters()
	{
		FNVirtualCellDataSummary Summary;
		Summary.bFoundStarterTagged = true;
		return Summary;
	}

	/** A summary whose finisher flags advertise that the pool contains end-specific content. */
	static FNVirtualCellDataSummary SummaryWithFinishers()
	{
		FNVirtualCellDataSummary Summary;
		Summary.bFoundFinisherTagged = true;
		return Summary;
	}
}

//
// Starter gate (FNVirtualOrganContext::IsGatedByStarterTags)
//

N_TEST_HIGH(FNStarterFinisherTagsTests_StarterTags_StartNodeRequiresStarterWhenPoolHasStarters,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::StarterTags::StartNodeRequiresStarterWhenPoolHasStarters",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the pool contains start-specific content, a start-node candidate must itself be Starter/StarterOnly;
	// an untagged candidate is gated out, while either starter tag passes.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;
	const FNVirtualCellDataSummary Summary = SummaryWithStarters();

	CHECK_MESSAGE(TEXT("An untagged candidate must be gated out of the start node when starter content exists."),
		FNVirtualOrganContext::IsGatedByStarterTags(true, Summary, Untagged()))
	CHECK_FALSE_MESSAGE(TEXT("A Starter candidate must be allowed at the start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(true, Summary, FGameplayTagContainer(Starter())))
	CHECK_FALSE_MESSAGE(TEXT("A StarterOnly candidate must be allowed at the start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(true, Summary, FGameplayTagContainer(StarterOnly())))
}

N_TEST_MEDIUM(FNStarterFinisherTagsTests_StarterTags_StartNodeAllowsUntaggedWhenNoStarterContent,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::StarterTags::StartNodeAllowsUntaggedWhenNoStarterContent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no start-specific content in the pool, the "must be a starter" requirement is not imposed, so any
	// untagged candidate may take the start node.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;

	CHECK_FALSE_MESSAGE(TEXT("Without starter content an untagged candidate must be allowed at the start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(true, FNVirtualCellDataSummary(), Untagged()))
}

N_TEST_HIGH(FNStarterFinisherTagsTests_StarterTags_StarterOnlyBarredFromNonStart,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::StarterTags::StarterOnlyBarredFromNonStart",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A StarterOnly cell may only ever be the start node; it must be gated out everywhere else and allowed there.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;

	CHECK_MESSAGE(TEXT("StarterOnly must be gated out of a non-start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(false, FNVirtualCellDataSummary(), FGameplayTagContainer(StarterOnly())))
	CHECK_FALSE_MESSAGE(TEXT("StarterOnly must be allowed at the start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(true, SummaryWithStarters(), FGameplayTagContainer(StarterOnly())))
}

N_TEST_HIGH(FNStarterFinisherTagsTests_StarterTags_NotStarterBarredFromStart,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::StarterTags::NotStarterBarredFromStart",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A NotStarter cell may never be the start node, but is free to be placed anywhere else.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;

	CHECK_MESSAGE(TEXT("NotStarter must be gated out of the start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(true, FNVirtualCellDataSummary(), FGameplayTagContainer(NotStarter())))
	CHECK_FALSE_MESSAGE(TEXT("NotStarter must be allowed at a non-start node."),
		FNVirtualOrganContext::IsGatedByStarterTags(false, FNVirtualCellDataSummary(), FGameplayTagContainer(NotStarter())))
}

//
// Finisher gate (FNVirtualOrganContext::IsGatedByFinisherTags) — mirror image of the starter gate.
//

N_TEST_HIGH(FNStarterFinisherTagsTests_FinisherTags_EndNodeRequiresFinisherWhenPoolHasFinishers,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherTags::EndNodeRequiresFinisherWhenPoolHasFinishers",
	N_TEST_CONTEXT_ANYWHERE)
{
	// When the pool contains end-specific content, an end-node candidate must itself be Finisher/FinisherOnly.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;
	const FNVirtualCellDataSummary Summary = SummaryWithFinishers();

	CHECK_MESSAGE(TEXT("An untagged candidate must be gated out of the end node when finisher content exists."),
		FNVirtualOrganContext::IsGatedByFinisherTags(true, Summary, Untagged()))
	CHECK_FALSE_MESSAGE(TEXT("A Finisher candidate must be allowed at the end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(true, Summary, FGameplayTagContainer(Finisher())))
	CHECK_FALSE_MESSAGE(TEXT("A FinisherOnly candidate must be allowed at the end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(true, Summary, FGameplayTagContainer(FinisherOnly())))
}

N_TEST_MEDIUM(FNStarterFinisherTagsTests_FinisherTags_EndNodeAllowsUntaggedWhenNoFinisherContent,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherTags::EndNodeAllowsUntaggedWhenNoFinisherContent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// With no end-specific content in the pool, any untagged candidate may take the end node.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;

	CHECK_FALSE_MESSAGE(TEXT("Without finisher content an untagged candidate must be allowed at the end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(true, FNVirtualCellDataSummary(), Untagged()))
}

N_TEST_HIGH(FNStarterFinisherTagsTests_FinisherTags_FinisherOnlyBarredFromNonEnd,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherTags::FinisherOnlyBarredFromNonEnd",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A FinisherOnly cell may only ever be the end node; it must be gated out everywhere else and allowed there.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;

	CHECK_MESSAGE(TEXT("FinisherOnly must be gated out of a non-end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(false, FNVirtualCellDataSummary(), FGameplayTagContainer(FinisherOnly())))
	CHECK_FALSE_MESSAGE(TEXT("FinisherOnly must be allowed at the end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(true, SummaryWithFinishers(), FGameplayTagContainer(FinisherOnly())))
}

N_TEST_HIGH(FNStarterFinisherTagsTests_FinisherTags_NotFinisherBarredFromEnd,
	"NEXUS::UnitTests::NWorldAssembly::FNVirtualOrganContext::FinisherTags::NotFinisherBarredFromEnd",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A NotFinisher cell may never be the end node, but is free to be placed anywhere else.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNStarterFinisherTagsHarness;

	CHECK_MESSAGE(TEXT("NotFinisher must be gated out of the end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(true, FNVirtualCellDataSummary(), FGameplayTagContainer(NotFinisher())))
	CHECK_FALSE_MESSAGE(TEXT("NotFinisher must be allowed at a non-end node."),
		FNVirtualOrganContext::IsGatedByFinisherTags(false, FNVirtualCellDataSummary(), FGameplayTagContainer(NotFinisher())))
}

#endif //WITH_TESTS
