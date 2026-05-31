// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"

class FNTestLatentCommand_WithBase : public IAutomationLatentCommand
{
public:
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