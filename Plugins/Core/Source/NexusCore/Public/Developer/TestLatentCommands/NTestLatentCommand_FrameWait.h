// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Misc/AutomationTest.h"

class FNTestLatentCommand_FrameWait : public IAutomationLatentCommand
{
public:
	explicit FNTestLatentCommand_FrameWait(const int32 FramesToWait)
		: TotalFramesToWait(FramesToWait)
	{
	}

	virtual ~FNTestLatentCommand_FrameWait() override
	{
	}

	virtual bool Update() override
	{
		if (ElapsedFrames < TotalFramesToWait)
		{
			ElapsedFrames++;
			return false;
		}
		ElapsedFrames = 0;
		return true;
	}

private:
	int32 ElapsedFrames = 0;
	int32 TotalFramesToWait;
};
