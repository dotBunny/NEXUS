// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

/**
 * Latent automation command that runs a static method against the shared test UWorld (FNTestUtils::Environment).
 *
 * Used by the world-based test/perf harness once FNTestLatentCommand_CreateWorld has built the environment.
 * Optionally wraps the call in an FGCScopeGuard so garbage collection cannot run mid-test, the standard choice
 * for performance tests. Use the _WithBase variant when the method must also assert.
 */
class FNTestLatentCommand_WorldTest : public IAutomationLatentCommand
{
public:
	/**
	 * @param StaticMethod Callable executed when the command ticks, receiving the shared test world.
	 * @param bDisableGC When true, suppresses garbage collection for the duration of the call.
	 */
	FNTestLatentCommand_WorldTest(TFunction<void(UWorld*)> StaticMethod, const bool bDisableGC)
		: TestFunctionality(StaticMethod)
		, bDisableGarbageCollection(bDisableGC)
	{
	}

	virtual ~FNTestLatentCommand_WorldTest() override
	{
	}

	virtual bool Update() override
	{
		if (bDisableGarbageCollection)
		{
			FGCScopeGuard Guard;
			{
				TestFunctionality(FNTestUtils::Environment.World);
			}
		}
		else
		{
			TestFunctionality(FNTestUtils::Environment.World);
		}
		return true;
	}

private:
	TFunction<void(UWorld*)> TestFunctionality;
	const bool bDisableGarbageCollection;
};

#endif // WITH_TESTS
