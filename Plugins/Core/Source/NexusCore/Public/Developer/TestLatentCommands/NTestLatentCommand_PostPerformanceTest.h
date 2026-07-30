// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"

/**
 * Latent automation command that cleans up after a timed performance test by forcing a garbage-collection pass.
 *
 * Runs once the timed region is finished so allocations made during the test do not leak into subsequent ones.
 * Paired with FNTestLatentCommand_PrePerformanceTest.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/developer/test-latent-commands/test-latent-command-post-performance-test/">FNTestLatentCommand_PostPerformanceTest</a>
 */
class FNTestLatentCommand_PostPerformanceTest : public IAutomationLatentCommand
{
public:
	virtual ~FNTestLatentCommand_PostPerformanceTest() override
	{
	}

	virtual bool Update() override
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		return true;
	}
};

#endif // WITH_TESTS