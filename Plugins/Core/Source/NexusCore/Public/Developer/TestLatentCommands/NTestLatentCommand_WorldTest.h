// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

class FNTestLatentCommand_WorldTest : public IAutomationLatentCommand
{
public:
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
