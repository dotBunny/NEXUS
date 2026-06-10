// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NLevelUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNLevelUtilsHarness
{
	// A leaf content folder in the test host project (TestProject/Content/Maps) that holds exactly one map. The
	// duplication bug only needs a single path with >= 1 map to reproduce, and the host project's own content is the
	// most stable fixture available — it keeps this NexusCore test free of any dependency on a sibling or sample plugin.
	static const FString MapsPath = TEXT("/Game/Maps");

	/** @return true if every entry in Names is unique. */
	static bool HasNoDuplicates(const TArray<FString>& Names)
	{
		TSet<FString> Seen;
		Seen.Reserve(Names.Num());
		for (const FString& Name : Names)
		{
			bool bAlreadyPresent = false;
			Seen.Add(Name, &bAlreadyPresent);
			if (bAlreadyPresent)
			{
				return false;
			}
		}
		return true;
	}
}

N_TEST_HIGH(FNLevelUtilsTests_GetAllMapNames_RepeatedPathDoesNotDuplicate,
	"NEXUS::UnitTests::NCore::FNLevelUtils::GetAllMapNames::RepeatedPathDoesNotDuplicate",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Regression for the UObjectLibrary accumulation bug. LoadAssetDataFromPath accumulates into the library and
	// GetAssetDataList snapshots the whole library, so harvesting inside the load loop re-added every earlier path's
	// maps on each iteration. Passing the same path twice is the minimal trigger: the old code returned each map
	// twice, the fix returns each once.
	using namespace NEXUS::UnitTests::NCore::FNLevelUtilsHarness;

	const TArray<FString> Single = FNLevelUtils::GetAllMapNames({ MapsPath });

	// Guard the fixture: if /Game/Maps ever loses its map (or the asset registry has not scanned it) fail loudly here,
	// otherwise the no-duplicate check below would pass vacuously on two empty arrays.
	if (Single.Num() < 1)
	{
		ADD_ERROR("Expected at least one map under /Game/Maps; fixture missing or asset registry not scanned.");
		return;
	}

	const TArray<FString> Doubled = FNLevelUtils::GetAllMapNames({ MapsPath, MapsPath });

	CHECK_EQUALS("Listing the same path twice must not change the count — paths must not accumulate duplicates.",
		Doubled.Num(), Single.Num())
	CHECK_MESSAGE(TEXT("GetAllMapNames must return no duplicate map names."), HasNoDuplicates(Doubled))
}

#endif //WITH_TESTS
