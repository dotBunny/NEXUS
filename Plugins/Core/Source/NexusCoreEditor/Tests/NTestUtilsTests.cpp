// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNTestUtilsTests, "NEXUS::UnitTests::NCore::FNTestUtils::EditorWorldLeak", N_TEST_CONTEXT_EDITOR)
{
	// The empty lambda is intentional: WorldTestChecked snapshots UObject counts before and after,
	// so this test verifies that PIE world create/teardown alone does not leak any UObjects.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
	});
}

#endif //WITH_TESTS
