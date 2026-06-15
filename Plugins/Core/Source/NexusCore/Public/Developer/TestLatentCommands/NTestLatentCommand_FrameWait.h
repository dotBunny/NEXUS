// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"

/**
 * Latent automation command that stalls the test queue for a fixed number of frames before completing.
 *
 * Returns false from Update until the requested frame count has elapsed, letting the engine tick in between —
 * useful when a test must wait for deferred engine work (spawning, streaming, physics) to settle.
 */
class FNTestLatentCommand_FrameWait : public IAutomationLatentCommand
{
public:
	/** @param FramesToWait Number of automation ticks to defer before the command reports completion. */
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

#endif // WITH_TESTS