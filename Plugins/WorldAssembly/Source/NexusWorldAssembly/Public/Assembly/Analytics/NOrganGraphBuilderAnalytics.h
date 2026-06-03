// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyTaskTimer.h"
#include "Types/NIterationCounter.h"

/**
 * Analytics record captured for a single FNOrganGraphBuilderTask invocation.
 *
 * Tracks the wall-clock cost of the build, the number of retry iterations consumed, and
 * a set of per-iteration counters describing how many candidate nodes were added or rejected
 * for each rejection reason. Used by FNAssemblyTaskAnalytics to produce the post-build report.
 */
struct FNOrganGraphBuilderAnalytics
{
	/** Human-readable name of the organ task this record describes. */
	FString Name;

	/** Wall-clock duration of the organ build (across all retry iterations). */
	FNWorldAssemblyTaskTimer Timer = FNWorldAssemblyTaskTimer();

	/** Total number of build iterations consumed (1 + retries). */
	int32 Iterations = 1;
	int32 IterationsIndex = 0;

	/** Count of null-terminator nodes added per iteration. */
	FNIterationCounter AddNullNodes;

	/** Count of cell nodes successfully added per iteration. */
	FNIterationCounter AddCellNodes;

	/** Candidate start nodes rejected because they fell outside the organ bounds. */
	FNIterationCounter DiscardOutOfBoundsStart;

	/** Candidate start nodes rejected because they collided with world geometry. */
	FNIterationCounter DiscardWorldCollidingStart;

	/** Candidate cell nodes rejected because they fell outside the organ bounds. */
	FNIterationCounter DiscardOutOfBoundsCellNode;

	/** Candidate cell nodes rejected because they intersected an existing graph node. */
	FNIterationCounter DiscardIntersectingCellNode;

	/** Candidate cell nodes rejected because they collided with world geometry. */
	FNIterationCounter DiscardWorldCollidingCellNode;

	/** Candidate cell nodes rejected because they collided with another organ's existing nodes. */
	FNIterationCounter DiscardExistingNodeWorldCollidingCellNode;
	
	/** Candidate cell nodes rejected because the finisher constraint required a finisher cell and this candidate was not one. */
	FNIterationCounter DiscardDueToNonFinisherConstraint;

	/** Number of graph branches closed off by placing a finisher cell. */
	FNIterationCounter CappedWithFinisher;
	
	TMap<int32, TArray<FString>> IterationMessages = {
		{ 0, TArray<FString>() }
	};

	/** Advance every contained counter to the next iteration slot and bump Iterations. */
	void NextIteration()
	{
		Iterations++;
		IterationsIndex++;
		IterationMessages.Add(IterationsIndex, TArray<FString>());
		
		
		AddNullNodes.NextIteration();
		AddCellNodes.NextIteration();

		DiscardOutOfBoundsStart.NextIteration();
		DiscardWorldCollidingStart.NextIteration();

		DiscardOutOfBoundsCellNode.NextIteration();
		DiscardIntersectingCellNode.NextIteration();
		DiscardWorldCollidingCellNode.NextIteration();
		DiscardExistingNodeWorldCollidingCellNode.NextIteration();
		DiscardDueToNonFinisherConstraint.NextIteration();
		CappedWithFinisher.NextIteration();
	}
};
