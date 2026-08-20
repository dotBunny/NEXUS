// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "PCG/Elements/NGetTargetPointsElement.h"

namespace NEXUS::UnitTests::NCore::FNGetTargetPointsElementHarness
{
	/** Tags standing in for a component that belongs to one marker set among several on the same actor. */
	static const TArray<FName> TaggedComponent = {FName(TEXT("Rim")), FName(TEXT("Depth"))};
}

using namespace NEXUS::UnitTests::NCore::FNGetTargetPointsElementHarness;

N_TEST_HIGH(FNGetTargetPointsElementTests_MatchesTagFilter_GathersEverythingWhenFilteringIsOff,
	"NEXUS::UnitTests::NCore::FNGetTargetPointsElement::MatchesTagFilter::GathersEverythingWhenFilteringIsOff",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The default path: an untagged component is still a target point.
	CHECK_MESSAGE(TEXT("An untagged component should be gathered when filtering is off."),
		FNGetTargetPointsElement::MatchesTagFilter(TArray<FName>(), false, FName(TEXT("Depth"))));
	CHECK_MESSAGE(TEXT("A tagged component should be gathered when filtering is off."),
		FNGetTargetPointsElement::MatchesTagFilter(TaggedComponent, false, FName(TEXT("Missing"))));
}

N_TEST_HIGH(FNGetTargetPointsElementTests_MatchesTagFilter_GathersOnlyTheMatchingTag,
	"NEXUS::UnitTests::NCore::FNGetTargetPointsElement::MatchesTagFilter::GathersOnlyTheMatchingTag",
	N_TEST_CONTEXT_ANYWHERE)
{
	// This is what lets one actor carry depth markers and rim markers side by side.
	CHECK_MESSAGE(TEXT("A component carrying the tag should be gathered."),
		FNGetTargetPointsElement::MatchesTagFilter(TaggedComponent, true, FName(TEXT("Depth"))));
	CHECK_FALSE_MESSAGE(TEXT("A component without the tag should be skipped."),
		FNGetTargetPointsElement::MatchesTagFilter(TaggedComponent, true, FName(TEXT("Missing"))));
	CHECK_FALSE_MESSAGE(TEXT("An untagged component should be skipped when filtering is on."),
		FNGetTargetPointsElement::MatchesTagFilter(TArray<FName>(), true, FName(TEXT("Depth"))));
}

N_TEST_MEDIUM(FNGetTargetPointsElementTests_MatchesTagFilter_EmptyTagGathersEverything,
	"NEXUS::UnitTests::NCore::FNGetTargetPointsElement::MatchesTagFilter::EmptyTagGathersEverything",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Ticking the filter box before typing a tag must not silently empty the node's output, which would
	// read as the node being broken rather than as a filter excluding everything.
	CHECK_MESSAGE(TEXT("An enabled filter with no tag should gather an untagged component."),
		FNGetTargetPointsElement::MatchesTagFilter(TArray<FName>(), true, NAME_None));
	CHECK_MESSAGE(TEXT("An enabled filter with no tag should gather a tagged component."),
		FNGetTargetPointsElement::MatchesTagFilter(TaggedComponent, true, NAME_None));
}

N_TEST_MEDIUM(FNGetTargetPointsElementTests_MatchesTagFilter_TagMatchingIsExact,
	"NEXUS::UnitTests::NCore::FNGetTargetPointsElement::MatchesTagFilter::TagMatchingIsExact",
	N_TEST_CONTEXT_ANYWHERE)
{
	// No prefix or substring matching: "Dep" must not pick up the "Depth" markers.
	CHECK_FALSE_MESSAGE(TEXT("A partial tag should not match."),
		FNGetTargetPointsElement::MatchesTagFilter(TaggedComponent, true, FName(TEXT("Dep"))));
}

#endif //WITH_TESTS
