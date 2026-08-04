// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyTaskTimer.h"

/**
 * Analytics record captured for the FNConnectJunctionsTask stage.
 *
 * Alongside the timer, the counters break down what happened to every candidate pair the pass considered. Which
 * rejection dominates is the useful signal when a layout connects up less than expected: a wall of length
 * rejections points at the spline-length budget, a wall of collision rejections at the routing settings or at the
 * layout simply being too dense.
 */
struct FNConnectJunctionsAnalytics
{
	/** Wall-clock duration of the matching pass. */
	FNWorldAssemblyTaskTimer Timer = FNWorldAssemblyTaskTimer();

	/** Junctions the graph builders left unmatched, and which this pass therefore considered. */
	int32 OpenJunctionCount = 0;

	/** Pairs that cleared the cheap gates (different cell, matching socket size, within range) and were routed. */
	int32 CandidatePairCount = 0;

	/** Pairs that produced a clear route and were recorded. */
	int32 AcceptedCount = 0;

	/** Pairs abandoned because no variant of their route fit inside the maximum spline length. */
	int32 RejectedByLengthCount = 0;

	/** Pairs abandoned because every variant of their route hit geometry. */
	int32 RejectedByCollisionCount = 0;

	/**
	 * Pairs abandoned because their two cells were already joined and Allow Multiple Cell Connections is off.
	 * @note Counted before any routing is attempted, so these cost nothing beyond the lookup — a high number here
	 *       means the layout mates densely, not that connectors are failing.
	 */
	int32 RejectedByExistingConnectionCount = 0;

	/** Detour variants attempted across every pair, including those that went on to fail. */
	int32 AvoidanceAttemptCount = 0;

	/** Pairs that only succeeded because a detour variant cleared where the direct route did not. */
	int32 AvoidanceSuccessCount = 0;

	/** Swept prisms retained as collision for subsequent pairs to route around. */
	int32 ConnectorHullCount = 0;
};
