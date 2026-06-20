// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

/**
 * Latent automation command that stabilizes the engine immediately before a timed performance test.
 *
 * Warms up stack walking, forces a garbage-collection pass, flushes pending logs, and streams in all resources so
 * that one-time costs do not contaminate the measured region. Runs ahead of the timed body; paired with
 * FNTestLatentCommand_PostPerformanceTest.
 */
class FNTestLatentCommand_PrePerformanceTest : public IAutomationLatentCommand
{
public:
	virtual ~FNTestLatentCommand_PrePerformanceTest() override
	{
	}

	virtual bool Update() override
	{
		// Ensure that stack walking is initialized prior to performance testing to avoid library loading, this happens once.
		FNTestUtils::Environment.InitializeStackWalking();

		// Force garbage collection to ensure a clean slate for performance testing
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

		// Force the logs to be written to disk prior to the operation
		GLog->Flush();
		FVisualLogger::Get().Flush();

		// Ensure everything has been loaded
		IStreamingManager::Get().StreamAllResources();
		return true;
	}
};

#endif // WITH_TESTS
