// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NEditorUtils.h"
#include "Macros/NTestMacros.h"
#include "Misc/ConfigCacheIni.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NCore::FNEditorUtilsStagingHarness
{
	static const TCHAR* StagingSection = TEXT("Staging");
	static const TCHAR* AllowedKey = TEXT("+AllowedConfigFiles");
	static const TCHAR* DisallowedKey = TEXT("+DisallowedConfigFiles");
	static const FString Relative = TEXT("NEXUS/Config/StagingTest.ini");

	/** @return true if the [Staging] array under Key contains Value. */
	static bool ListContains(FConfigFile& ConfigFile, const TCHAR* Key, const FString& Value)
	{
		TArray<FString> Values;
		ConfigFile.GetArray(StagingSection, Key, Values);
		return Values.Contains(Value);
	}
}

N_TEST_HIGH(FNEditorUtilsTests_ApplyStagingConfigEntry_AddsToTargetList,
	"NEXUS::UnitTests::NCore::FNEditorUtils::ApplyStagingConfigEntry::AddsToTargetList",
	N_TEST_CONTEXT_ANYWHERE)
{
	using namespace NEXUS::UnitTests::NCore::FNEditorUtilsStagingHarness;

	FConfigFile ConfigFile;
	const bool bModified = FNEditorUtils::ApplyStagingConfigEntry(ConfigFile, Relative, DisallowedKey, AllowedKey);

	CHECK_MESSAGE(TEXT("Adding to an empty config must report a modification."), bModified)
	CHECK_MESSAGE(TEXT("Config must be present in the disallowed list after a disallow."),
		ListContains(ConfigFile, DisallowedKey, Relative))
	CHECK_FALSE_MESSAGE(TEXT("Config must not appear in the allowed list after a disallow."),
		ListContains(ConfigFile, AllowedKey, Relative))
}

N_TEST_HIGH(FNEditorUtilsTests_ApplyStagingConfigEntry_PrunesOpposingList,
	"NEXUS::UnitTests::NCore::FNEditorUtils::ApplyStagingConfigEntry::PrunesOpposingList",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Regression for the allow/disallow semantic gap: a config flipped from disallowed to allowed must end
	// up in exactly one list, never both.
	using namespace NEXUS::UnitTests::NCore::FNEditorUtilsStagingHarness;

	FConfigFile ConfigFile;
	FNEditorUtils::ApplyStagingConfigEntry(ConfigFile, Relative, DisallowedKey, AllowedKey);          // disallow
	const bool bModified = FNEditorUtils::ApplyStagingConfigEntry(ConfigFile, Relative, AllowedKey, DisallowedKey); // allow

	CHECK_MESSAGE(TEXT("Flipping a config to the opposing list must report a modification."), bModified)
	CHECK_MESSAGE(TEXT("Config must be present in the allowed list after an allow."),
		ListContains(ConfigFile, AllowedKey, Relative))
	CHECK_FALSE_MESSAGE(TEXT("Allow must prune the config from the disallowed list (no lingering in both)."),
		ListContains(ConfigFile, DisallowedKey, Relative))
}

N_TEST_MEDIUM(FNEditorUtilsTests_ApplyStagingConfigEntry_IsIdempotent,
	"NEXUS::UnitTests::NCore::FNEditorUtils::ApplyStagingConfigEntry::IsIdempotent",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The disk path only flushes when modified; a repeat call with no net change must report false so the
	// caller can skip the flush.
	using namespace NEXUS::UnitTests::NCore::FNEditorUtilsStagingHarness;

	FConfigFile ConfigFile;
	FNEditorUtils::ApplyStagingConfigEntry(ConfigFile, Relative, DisallowedKey, AllowedKey);
	const bool bModifiedAgain = FNEditorUtils::ApplyStagingConfigEntry(ConfigFile, Relative, DisallowedKey, AllowedKey);

	CHECK_FALSE_MESSAGE(TEXT("Re-disallowing an already-disallowed config must report no modification."), bModifiedAgain)
}

#endif //WITH_TESTS
