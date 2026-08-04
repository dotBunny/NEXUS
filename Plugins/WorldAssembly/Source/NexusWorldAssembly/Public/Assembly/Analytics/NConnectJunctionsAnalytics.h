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

	/**
	 * Unmatched junctions skipped for having Disable Connecting authored on them.
	 * @note Excluded from OpenJunctionCount, so that stays a count of what this pass could actually work with — this
	 *       is what explains the difference when a layout connects up less than the open sockets suggest.
	 */
	int32 DisabledJunctionCount = 0;

	/** Pairs that cleared the cheap gates (different cell, matching socket size, within range) and were routed. */
	int32 CandidatePairCount = 0;

	/** Pairs that produced a clear route and were recorded. */
	int32 AcceptedCount = 0;

	/** Pairs abandoned because no variant of their route fit inside the maximum spline length. */
	int32 RejectedByLengthCount = 0;

	/** Pairs abandoned because every variant of their route hit geometry. */
	int32 RejectedByCollisionCount = 0;

	/** Pairs abandoned because no variant of their route turned gently enough for the configured minimum radius. */
	int32 RejectedByTurnRadiusCount = 0;

	/**
	 * Pairs whose tightest variant would have folded the connector's geometry through itself.
	 * @note A subset of the turn-radius rejections, broken out because it is a validity failure rather than a tuning
	 *       one — these are rejected no matter how the minimum radius is configured.
	 */
	int32 RejectedByFoldCount = 0;

	/** Straighter variants attempted across every pair, including those that went on to fail anyway. */
	int32 StraighteningAttemptCount = 0;

	/** Pairs that only succeeded because a straighter variant cleared where the configured tangent scale did not. */
	int32 StraighteningSuccessCount = 0;

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
