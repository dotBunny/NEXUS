// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"

/**
 * Latent automation command that invokes a static method once, passing the owning FAutomationTestBase.
 *
 * Same deferral behavior as FNTestLatentCommand, but the test base is forwarded so the deferred method can call
 * the assertion macros (which expand to expressions referencing a `Test` symbol).
 */
class FNTestLatentCommand_WithBase : public IAutomationLatentCommand
{
public:
	/**
	 * @param StaticMethod Callable executed when the command ticks, receiving the test base.
	 * @param TestPtr The automation test the deferred method asserts against.
	 */
	FNTestLatentCommand_WithBase(TFunction<void(FAutomationTestBase*)> StaticMethod, FAutomationTestBase* TestPtr)
		: TestFunctionality(StaticMethod)
		, Test(TestPtr)
	{
	}

	virtual ~FNTestLatentCommand_WithBase() override
	{
	}

	virtual bool Update() override
	{
		TestFunctionality(Test);
		return true;
	}

private:
	TFunction<void(FAutomationTestBase*)> TestFunctionality;
	FAutomationTestBase* Test;
};

#endif // WITH_TESTS