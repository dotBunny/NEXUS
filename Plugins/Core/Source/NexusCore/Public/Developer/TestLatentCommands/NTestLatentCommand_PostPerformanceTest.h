// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/AutomationTest.h"

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