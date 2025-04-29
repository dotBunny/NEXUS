// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_CRITICAL(FNBaseTests, "NEXUS::UnitTests::NCore::EditorWorldLeak", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
	});
}

#endif //WITH_TESTS
