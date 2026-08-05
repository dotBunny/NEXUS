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
	 * Unmatched junctions carrying Disable Connecting, which the routing walk therefore skipped.
	 * @note A subset of OpenJunctionCount rather than a deduction from it: these junctions remain eligible for
	 *       inverse matching, since that spawns no connector for them to have opted out of.
	 */
	int32 DisabledJunctionCount = 0;

	/**
	 * Junctions mated to a coincident, oppositely-facing partner rather than routed to one, counted per pairing.
	 * @note These never reach the routing walk at all, so they are absent from CandidatePairCount and from every
	 *       rejection counter below — a layout that suddenly connects up better after enabling Connect Inverse shows
	 *       it here rather than in AcceptedCount.
	 */
	int32 InverseMatchCount = 0;

	/** Pairs that cleared the cheap gates (different cell, matching socket size, within range) and were routed. */
	int32 CandidatePairCount = 0;

	/**
	 * Pairs discarded because the two junctions were not oriented sensibly enough with respect to each other.
	 * @note The only rejection counted before a pair becomes a candidate, so unlike every counter below it these are
	 *       absent from CandidatePairCount rather than a breakdown of it. Broken out because a gate that silently
	 *       halves the candidate count is otherwise invisible: a large number here against few acceptances points at
	 *       the angle limits, not at the routing budget.
	 */
	int32 RejectedByAngleCount = 0;

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
