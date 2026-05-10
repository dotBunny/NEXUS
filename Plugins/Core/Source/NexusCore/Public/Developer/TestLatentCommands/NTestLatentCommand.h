// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/AutomationTest.h"

class FNTestLatentCommand : public IAutomationLatentCommand
{
public:
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
