// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyTaskTimer.h"

/**
 * Analytics record captured for the FNEvaluateGraphsTask stage.
 *
 * The stage runs three passes over the finished graphs and each is timed separately, because they do not scale
 * alike and the aggregate hides which one is responsible. Hot path resolution runs a breadth-first search per goal
 * for the shortest variant plus a chained one per goal for the sequential variant, so it grows with goal count as
 * well as graph size and is very nearly always the dominant cost. Proximity scoring is three sweeps for the whole
 * operation regardless of how many goals there are, and link details are a single linear walk.
 *
 * The counts alongside the timers are what make those durations readable: hot path time is only interpretable
 * against the number of goals that produced it.
 */
struct FNEvaluateGraphsAnalytics
{
	/** Wall-clock duration of the whole stage. */
	FNWorldAssemblyTaskTimer Timer = FNWorldAssemblyTaskTimer();

	/** Wall-clock duration of hot path resolution across every graph. */
	FNWorldAssemblyTaskTimer HotPathTimer = FNWorldAssemblyTaskTimer();

	/** Wall-clock duration of the proximity scoring sweeps. */
	FNWorldAssemblyTaskTimer ProximityTimer = FNWorldAssemblyTaskTimer();

	/** Wall-clock duration of link-detail generation across every cell. */
	FNWorldAssemblyTaskTimer LinkDetailsTimer = FNWorldAssemblyTaskTimer();

	/** Graphs the stage evaluated. */
	int32 GraphCount = 0;

	/** Cell nodes across those graphs, which is what link-detail generation walked. */
	int32 CellCount = 0;

	/**
	 * Cells tagged Hotpath across every graph — the goals hot path resolution had to thread a route through.
	 * @note The multiplier on hot path cost rather than a result of it. Zero means there is no hot path at all, and
	 *       every hot path score in the operation will read as unreachable.
	 */
	int32 HotPathGoalCount = 0;

	/** Cells tagged Important across every graph, which seeded the importance score. */
	int32 ImportantCellCount = 0;
};
