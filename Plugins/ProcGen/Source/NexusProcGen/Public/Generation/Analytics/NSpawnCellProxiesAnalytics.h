// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenTaskTimer.h"

/**
 * Analytics record captured for a single FNSpawnCellProxiesTask invocation.
 *
 * Records every cell template that was spawned during the pass plus the wall-clock cost of
 * the spawn step, for inclusion in the analytics report.
 */
struct FNSpawnCellProxiesAnalytics
{
	/** Cell template names spawned during this task, in spawn order. */
	TArray<FName> Spawned;

	/** Wall-clock duration of the spawn step. */
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
};
