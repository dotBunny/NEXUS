// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenTaskTimer.h"

/**
 * Analytics record captured for a single FNProcessPassTask invocation.
 *
 * The phase counter identifies which generation pass the task belonged to, and the timer
 * tracks how long the pass-collection step took to drain its inputs into the task-graph context.
 */
struct FNProcessPassAnalytics
{
	/** Index of the generation pass this record corresponds to. */
	int32 Phase = 0;

	/** Wall-clock duration of the pass-collection step. */
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
};
