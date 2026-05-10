// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

class FNTestLatentCommand_WorldTestWithBase : public IAutomationLatentCommand
{
public:
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
