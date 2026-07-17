// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblyContextCache.h"

#include "Collections/NGameplayTagCounter.h"
#include "GameplayTagContainer.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness
{
	// The cache only cares about a tag's identity as a map key, so any two registered tags will do. We reuse the
	// built-in behavior tags (registered in NWorldAssemblyGameplayTags.cpp) purely as two distinct identities.
	static FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	static FGameplayTag TagA() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Starter")); }
	static FGameplayTag TagB() { return Tag(TEXT("NEXUS.WorldAssembly.Behavior.Finisher")); }

	/** Builds a counter holding a single tag at the given count. */
	static FNGameplayTagCounter MakeCounter(const FGameplayTag& InTag, const int32 Count)
	{
		FNGameplayTagCounter Counter;
		Counter.Add(InTag, Count);
		return Counter;
	}

	/** Builds a container holding the supplied tags. */
	static FGameplayTagContainer MakeContainer(const TArray<FGameplayTag>& Tags)
	{
		FGameplayTagContainer Container;
		for (const FGameplayTag& InTag : Tags)
		{
			Container.AddTag(InTag);
		}
		return Container;
	}

	// The cache is process-global static state, so every test uses a ticket unique to that test and removes it
	// at the end to keep tests independent of one another and of execution order.
	constexpr int32 LifecycleTicket = 90001;
	constexpr int32 TryGetTagCountTicket = 90002;
	constexpr int32 HasTagCountTicket = 90003;
	constexpr int32 MutateTagCountTicket = 90004;
	constexpr int32 AbsentOperationTicket = 90005;
	constexpr int32 HasContextTagsTicket = 90006;
	constexpr int32 MutateContextTagsTicket = 90007;
	constexpr int32 OverwriteTicket = 90008;
	constexpr int32 ClearTicketA = 90009;
	constexpr int32 ClearTicketB = 90010;
	constexpr int32 ClearTicketC = 90011;
}

N_TEST_CRITICAL(FNWorldAssemblyContextCacheTests_Lifecycle_AddHasRemove,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::Lifecycle::AddHasRemove",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A ticket is absent until added, present once cached, and absent again after removal.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	CHECK_FALSE_MESSAGE(TEXT("An unregistered ticket must not be reported as present."),
		FNWorldAssemblyContextCache::HasOperation(LifecycleTicket))

	FNWorldAssemblyContextCache::AddOperationContext(LifecycleTicket, MakeCounter(TagA(), 2), MakeContainer({ TagA() }));

	CHECK_MESSAGE(TEXT("A ticket must be present after AddOperationContext."),
		FNWorldAssemblyContextCache::HasOperation(LifecycleTicket))

	FNWorldAssemblyContextCache::RemoveOperationContext(LifecycleTicket);

	CHECK_FALSE_MESSAGE(TEXT("A ticket must be absent after RemoveOperationContext."),
		FNWorldAssemblyContextCache::HasOperation(LifecycleTicket))
}

N_TEST_HIGH(FNWorldAssemblyContextCacheTests_TagCounter_TryGetTagCount,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::TagCounter::TryGetTagCount",
	N_TEST_CONTEXT_ANYWHERE)
{
	// TryGetTagCount writes the count only when both the operation and the tag are present.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	int32 Count = -1;
	CHECK_FALSE_MESSAGE(TEXT("TryGetTagCount must fail for an unregistered operation."),
		FNWorldAssemblyContextCache::TryGetTagCount(TryGetTagCountTicket, TagA(), Count))
	CHECK_EQUALS("TryGetTagCount must leave OutCount untouched when it fails.", Count, -1)

	FNWorldAssemblyContextCache::AddOperationContext(TryGetTagCountTicket, MakeCounter(TagA(), 5), FGameplayTagContainer());

	CHECK_MESSAGE(TEXT("TryGetTagCount must succeed for a present tag."),
		FNWorldAssemblyContextCache::TryGetTagCount(TryGetTagCountTicket, TagA(), Count))
	CHECK_EQUALS("TryGetTagCount must write the cached count.", Count, 5)

	Count = -1;
	CHECK_FALSE_MESSAGE(TEXT("TryGetTagCount must fail for a tag the counter does not track."),
		FNWorldAssemblyContextCache::TryGetTagCount(TryGetTagCountTicket, TagB(), Count))
	CHECK_EQUALS("TryGetTagCount must leave OutCount untouched for an absent tag.", Count, -1)

	FNWorldAssemblyContextCache::RemoveOperationContext(TryGetTagCountTicket);
}

N_TEST_MEDIUM(FNWorldAssemblyContextCacheTests_TagCounter_HasTagCount,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::TagCounter::HasTagCount",
	N_TEST_CONTEXT_ANYWHERE)
{
	// HasTagCount is true only when the operation is cached and its counter tracks the tag.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	CHECK_FALSE_MESSAGE(TEXT("HasTagCount must be false for an unregistered operation."),
		FNWorldAssemblyContextCache::HasTagCount(HasTagCountTicket, TagA()))

	FNWorldAssemblyContextCache::AddOperationContext(HasTagCountTicket, MakeCounter(TagA(), 1), FGameplayTagContainer());

	CHECK_MESSAGE(TEXT("HasTagCount must be true for a tracked tag."),
		FNWorldAssemblyContextCache::HasTagCount(HasTagCountTicket, TagA()))
	CHECK_FALSE_MESSAGE(TEXT("HasTagCount must be false for an untracked tag."),
		FNWorldAssemblyContextCache::HasTagCount(HasTagCountTicket, TagB()))

	FNWorldAssemblyContextCache::RemoveOperationContext(HasTagCountTicket);
}

N_TEST_HIGH(FNWorldAssemblyContextCacheTests_TagCounter_AddAndSubtractMutateCachedCounter,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::TagCounter::AddAndSubtractMutateCachedCounter",
	N_TEST_CONTEXT_ANYWHERE)
{
	// AddTagCount/SubtractTagCount mutate the cached counter in place and the change persists across reads.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	FNWorldAssemblyContextCache::AddOperationContext(MutateTagCountTicket, FNGameplayTagCounter(), FGameplayTagContainer());

	FNWorldAssemblyContextCache::AddTagCount(MutateTagCountTicket, TagA(), 3);
	int32 Count = -1;
	FNWorldAssemblyContextCache::TryGetTagCount(MutateTagCountTicket, TagA(), Count);
	CHECK_EQUALS("AddTagCount must seed the count from zero.", Count, 3)

	FNWorldAssemblyContextCache::AddTagCount(MutateTagCountTicket, TagA(), 2);
	FNWorldAssemblyContextCache::TryGetTagCount(MutateTagCountTicket, TagA(), Count);
	CHECK_EQUALS("AddTagCount must accumulate onto the existing count.", Count, 5)

	FNWorldAssemblyContextCache::SubtractTagCount(MutateTagCountTicket, TagA(), 1);
	FNWorldAssemblyContextCache::TryGetTagCount(MutateTagCountTicket, TagA(), Count);
	CHECK_EQUALS("SubtractTagCount must reduce the existing count.", Count, 4)

	FNWorldAssemblyContextCache::RemoveOperationContext(MutateTagCountTicket);
}

N_TEST_HIGH(FNWorldAssemblyContextCacheTests_AbsentOperation_MutatorsAreNoOps,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::AbsentOperation::MutatorsAreNoOps",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Mutating an operation that was never cached must not create an entry — the maps stay empty for that ticket.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	FNWorldAssemblyContextCache::AddTagCount(AbsentOperationTicket, TagA(), 5);
	FNWorldAssemblyContextCache::SubtractTagCount(AbsentOperationTicket, TagA(), 1);
	FNWorldAssemblyContextCache::AppendContextTags(AbsentOperationTicket, MakeContainer({ TagA() }));
	FNWorldAssemblyContextCache::RemoveContextTags(AbsentOperationTicket, MakeContainer({ TagA() }));

	CHECK_FALSE_MESSAGE(TEXT("Mutating an absent operation must not register it."),
		FNWorldAssemblyContextCache::HasOperation(AbsentOperationTicket))

	int32 Count = -1;
	CHECK_FALSE_MESSAGE(TEXT("An absent operation must have no tag count after mutation attempts."),
		FNWorldAssemblyContextCache::TryGetTagCount(AbsentOperationTicket, TagA(), Count))
	CHECK_FALSE_MESSAGE(TEXT("An absent operation must have no context tags after mutation attempts."),
		FNWorldAssemblyContextCache::HasContextTags(AbsentOperationTicket, MakeContainer({ TagA() })))
}

N_TEST_HIGH(FNWorldAssemblyContextCacheTests_ContextTags_HasContextTagsRequiresAllExact,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::ContextTags::HasContextTagsRequiresAllExact",
	N_TEST_CONTEXT_ANYWHERE)
{
	// HasContextTags is true only when every queried tag is present, and false for an unregistered operation.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	CHECK_FALSE_MESSAGE(TEXT("HasContextTags must be false for an unregistered operation."),
		FNWorldAssemblyContextCache::HasContextTags(HasContextTagsTicket, MakeContainer({ TagA() })))

	FNWorldAssemblyContextCache::AddOperationContext(HasContextTagsTicket, FNGameplayTagCounter(), MakeContainer({ TagA() }));

	CHECK_MESSAGE(TEXT("HasContextTags must be true when the queried tag is present."),
		FNWorldAssemblyContextCache::HasContextTags(HasContextTagsTicket, MakeContainer({ TagA() })))
	CHECK_FALSE_MESSAGE(TEXT("HasContextTags must be false when any queried tag is missing."),
		FNWorldAssemblyContextCache::HasContextTags(HasContextTagsTicket, MakeContainer({ TagA(), TagB() })))

	FNWorldAssemblyContextCache::RemoveOperationContext(HasContextTagsTicket);
}

N_TEST_HIGH(FNWorldAssemblyContextCacheTests_ContextTags_AppendAndRemoveMutateCachedContainer,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::ContextTags::AppendAndRemoveMutateCachedContainer",
	N_TEST_CONTEXT_ANYWHERE)
{
	// AppendContextTags/RemoveContextTags mutate the cached container in place and the change persists across reads.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	FNWorldAssemblyContextCache::AddOperationContext(MutateContextTagsTicket, FNGameplayTagCounter(), FGameplayTagContainer());

	FNWorldAssemblyContextCache::AppendContextTags(MutateContextTagsTicket, MakeContainer({ TagA(), TagB() }));
	CHECK_MESSAGE(TEXT("AppendContextTags must add both tags to the cached container."),
		FNWorldAssemblyContextCache::HasContextTags(MutateContextTagsTicket, MakeContainer({ TagA(), TagB() })))

	FNWorldAssemblyContextCache::RemoveContextTags(MutateContextTagsTicket, MakeContainer({ TagB() }));
	CHECK_FALSE_MESSAGE(TEXT("RemoveContextTags must drop the removed tag."),
		FNWorldAssemblyContextCache::HasContextTags(MutateContextTagsTicket, MakeContainer({ TagB() })))
	CHECK_MESSAGE(TEXT("RemoveContextTags must leave the untouched tag in place."),
		FNWorldAssemblyContextCache::HasContextTags(MutateContextTagsTicket, MakeContainer({ TagA() })))

	FNWorldAssemblyContextCache::RemoveOperationContext(MutateContextTagsTicket);
}

N_TEST_MEDIUM(FNWorldAssemblyContextCacheTests_Lifecycle_AddOverwritesExistingSnapshot,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::Lifecycle::AddOverwritesExistingSnapshot",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A second AddOperationContext for the same ticket replaces the prior snapshot rather than merging.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	FNWorldAssemblyContextCache::AddOperationContext(OverwriteTicket, MakeCounter(TagA(), 1), MakeContainer({ TagA() }));
	FNWorldAssemblyContextCache::AddOperationContext(OverwriteTicket, MakeCounter(TagA(), 9), MakeContainer({ TagB() }));

	int32 Count = -1;
	FNWorldAssemblyContextCache::TryGetTagCount(OverwriteTicket, TagA(), Count);
	CHECK_EQUALS("The second AddOperationContext must overwrite the tag count.", Count, 9)

	CHECK_MESSAGE(TEXT("The second AddOperationContext must overwrite the context tags."),
		FNWorldAssemblyContextCache::HasContextTags(OverwriteTicket, MakeContainer({ TagB() })))
	CHECK_FALSE_MESSAGE(TEXT("The overwritten context tags must no longer contain the original tag."),
		FNWorldAssemblyContextCache::HasContextTags(OverwriteTicket, MakeContainer({ TagA() })))

	FNWorldAssemblyContextCache::RemoveOperationContext(OverwriteTicket);
}

N_TEST_HIGH(FNWorldAssemblyContextCacheTests_Lifecycle_ClearContextRemovesOnlyListedTickets,
	"NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCache::Lifecycle::ClearContextRemovesOnlyListedTickets",
	N_TEST_CONTEXT_ANYWHERE)
{
	// ClearContext drops every listed ticket (skipping unregistered ones) and leaves unlisted tickets intact.
	using namespace NEXUS::UnitTests::NWorldAssembly::FNWorldAssemblyContextCacheHarness;

	FNWorldAssemblyContextCache::AddOperationContext(ClearTicketA, MakeCounter(TagA(), 1), MakeContainer({ TagA() }));
	FNWorldAssemblyContextCache::AddOperationContext(ClearTicketB, MakeCounter(TagA(), 1), MakeContainer({ TagA() }));

	// ClearTicketC is never registered, so it exercises the skip-absent path alongside the two live tickets.
	FNWorldAssemblyContextCache::ClearContext({ ClearTicketA, ClearTicketC });

	CHECK_FALSE_MESSAGE(TEXT("ClearContext must remove a listed, registered ticket."),
		FNWorldAssemblyContextCache::HasOperation(ClearTicketA))
	CHECK_MESSAGE(TEXT("ClearContext must leave a ticket that was not listed intact."),
		FNWorldAssemblyContextCache::HasOperation(ClearTicketB))

	FNWorldAssemblyContextCache::RemoveOperationContext(ClearTicketB);
}

#endif //WITH_TESTS
