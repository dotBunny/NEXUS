// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"

/**
 * Latent automation command that invokes a parameterless static method once on the next automation tick.
 *
 * The base building block for deferring test work until after the engine has ticked; used by the N_TEST_PERF
 * macros and any test that needs its body to run outside the immediate registration call. Use the _WithBase
 * variant when the deferred method must raise assertions through an FAutomationTestBase.
 */
class FNTestLatentCommand : public IAutomationLatentCommand
{
public:
	/** @param StaticMethod Parameterless callable executed when the command ticks. */
	FNTestLatentCommand(TFunction<void()> StaticMethod)
		: TestFunctionality(StaticMethod)
	{
	}

	virtual ~FNTestLatentCommand() override
	{
	}

	virtual bool Update() override
	{
		TestFunctionality();
		return true;
	}

private:
	TFunction<void()> TestFunctionality;
};

#endif // WITH_TESTS
