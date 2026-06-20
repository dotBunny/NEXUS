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

	/** Whether the build ultimately produced a graph that passed validation. */
	bool bSucceeded = false;

	/** Cell-node count of the produced graph; on failure, the count from the final attempt. */
	int32 FinalCellNodeCount = 0;

	/** Lower bound on cell count required for success, captured from the organ context; -1 disables the check. */
	int32 MinimumCellCount = -1;

	/** Upper bound on cell count required for success, captured from the organ context; -1 disables the check. */
	int32 MaximumCellCount = -1;

	/** Maximum retries the builder was permitted before giving up. */
	int32 MaximumRetryCount = 0;

	/** Explicit failure reason recorded at an early bailout that never reaches graph validation; preferred over the message-derived reason. */
	FString FailureReasonOverride;

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

	/** @return A concise pass/fail tag for table cells: "OK" on success, otherwise "FAILED (got N)". */
	FString GetStatusString() const
	{
		return bSucceeded ? FString(TEXT("OK")) : FString::Printf(TEXT("FAILED (got %i)"), FinalCellNodeCount);
	}

	/**
	 * Recover the most specific validation failure reason for the final attempt.
	 * @return The last iteration's "CheckGraph FAILED:" message when present, the last logged message otherwise,
	 * or an empty string when the build succeeded or recorded no messages.
	 */
	FString GetFailureReason() const
	{
		if (bSucceeded) return FString();

		// An explicit reason recorded at an early bailout is more specific than anything in the message log.
		if (!FailureReasonOverride.IsEmpty()) return FailureReasonOverride;

		const TArray<FString>* LastMessages = IterationMessages.Find(Iterations - 1);
		if (LastMessages == nullptr || LastMessages->IsEmpty()) return FString();

		// Prefer the detailed CheckGraph diagnosis over the generic "ValidateGraph FAILED!" line.
		for (int32 i = LastMessages->Num() - 1; i >= 0; i--)
		{
			if ((*LastMessages)[i].Contains(TEXT("CheckGraph FAILED")))
			{
				return (*LastMessages)[i];
			}
		}
		return LastMessages->Last();
	}

	/**
	 * Identify the rejection reason that discarded the most candidate cells across every iteration. Start-node and
	 * cell-node variants of the same physical reason (world collision, out-of-bounds) are summed together.
	 * @param OutReason [out] Human-readable name of the dominant rejection reason; untouched when nothing was rejected.
	 * @param OutRejectCount [out] Number of candidates discarded for the dominant reason.
	 * @param OutTotalAttempts [out] Total placement attempts (all rejections plus successful cell additions).
	 * @return true when at least one candidate was rejected and a dominant reason was identified.
	 */
	bool GetDominantRejection(FString& OutReason, int32& OutRejectCount, int32& OutTotalAttempts) const
	{
		auto Sum = [](const FNIterationCounter& Counter)
		{
			int32 Total = 0;
			for (const int32 Value : Counter.Counter)
			{
				Total += Value;
			}
			return Total;
		};

		const TPair<FString, int32> Rejections[] = {
			{ TEXT("World Colliding"),         Sum(DiscardWorldCollidingCellNode) + Sum(DiscardWorldCollidingStart) },
			{ TEXT("Existing Node Colliding"), Sum(DiscardExistingNodeWorldCollidingCellNode) },
			{ TEXT("Intersecting Cell"),       Sum(DiscardIntersectingCellNode) },
			{ TEXT("Out Of Bounds"),           Sum(DiscardOutOfBoundsCellNode) + Sum(DiscardOutOfBoundsStart) },
			{ TEXT("Non-Finisher Constraint"), Sum(DiscardDueToNonFinisherConstraint) }
		};

		int32 TotalRejections = 0;
		OutRejectCount = 0;
		for (const TPair<FString, int32>& Rejection : Rejections)
		{
			TotalRejections += Rejection.Value;
			if (Rejection.Value > OutRejectCount)
			{
				OutRejectCount = Rejection.Value;
				OutReason = Rejection.Key;
			}
		}

		OutTotalAttempts = TotalRejections + Sum(AddCellNodes);
		return OutRejectCount > 0;
	}
};
