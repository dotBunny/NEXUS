// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

/**
 * Latent automation command that runs a static method against the shared test UWorld and forwards the test base.
 *
 * Combines FNTestLatentCommand_WorldTest's world access and optional GC suppression with the assertion support of
 * FNTestLatentCommand_WithBase, for world-based tests whose deferred method needs to raise assertions.
 */
class FNTestLatentCommand_WorldTestWithBase : public IAutomationLatentCommand
{
public:
	/**
	 * @param StaticMethod Callable executed when the command ticks, receiving the shared test world and test base.
	 * @param bDisableGC When true, suppresses garbage collection for the duration of the call.
	 * @param TestPtr The automation test the deferred method asserts against.
	 */
	FNTestLatentCommand_WorldTestWithBase(TFunction<void(UWorld*, FAutomationTestBase*)> StaticMethod, const bool bDisableGC, FAutomationTestBase* TestPtr)
		: TestFunctionality(StaticMethod)
		, bDisableGarbageCollection(bDisableGC)
		, Test(TestPtr)
	{
	}

	virtual ~FNTestLatentCommand_WorldTestWithBase() override
	{
	}

	virtual bool Update() override
	{
		if (bDisableGarbageCollection)
		{
			FGCScopeGuard Guard;
			{
				TestFunctionality(FNTestUtils::Environment.World, Test);
			}
		}
		else
		{
			TestFunctionality(FNTestUtils::Environment.World, Test);
		}
		return true;
	}

private:
	TFunction<void(UWorld*, FAutomationTestBase*)> TestFunctionality;
	const bool bDisableGarbageCollection;
	FAutomationTestBase* Test;
};

#endif // WITH_TESTS
