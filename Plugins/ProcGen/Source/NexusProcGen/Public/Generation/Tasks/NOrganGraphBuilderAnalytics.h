// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenMinimal.h"

/**
 * Counters gathered while a single organ's graph is being built, for logging and retry diagnostics.
 *
 * Current-attempt counters (CellNodes/NullNodes) are reset on Retry; Total* variants accumulate
 * across the whole build so post-mortem logs show both the final attempt and the overall effort.
 */
struct FNOrganGraphBuilderAnalytics
{
	/** Record a successful null-node placement. */
	void AddedNullNode()
	{
		NullNodes++;
		TotalNullNodes++;
	}

	/** Record a successful cell-node placement. */
	void AddedCellNode()
	{
		CellNodes++;
		TotalCellNodes++;
	}

	/** Record a candidate discarded because it fell outside the organ bounds. */
	void DiscardOutOfBounds()
	{
		DiscardedOutOfBoundsNodes++;
	}

	/** Record a candidate discarded because it intersected an existing node. */
	void DiscardIntersecting()
	{
		DiscardedIntersectingNodes++;
	}

	/** Record a retry — bumps RetryCount and resets current-attempt counters. */
	void Retry()
	{
		RetryCount++;
		CellNodes = 0;
		NullNodes = 0;
	}

	/** Capture the organ's display name and input constraints for use in OutputToLog. */
	void Init(FString Name, const int MinimumCells, const int MaximumCells, const int MaximumRetries)
	{
		DisplayName = Name;
		InputMinimumCells = MinimumCells;
		InputMaximumCells = MaximumCells;
		InputMaximumRetries = MaximumRetries;
	}

	/** Emit the collected analytics to LogNexusProcGen. */
	void OutputToLog() const
	{
		FStringBuilderBase Builder = FStringBuilderBase();

		Builder.Appendf(TEXT("\n[FNOrganGeneratorTaskAnalytics] %s\n"), *DisplayName);
		Builder.Append(TEXT("\tResets:\n"));
		Builder.Appendf(TEXT("\t\tRetry Count: %i/%i\n"), RetryCount, InputMaximumRetries);
		Builder.Append(TEXT("\tCreated Nodes:\n"));
		Builder.Appendf(TEXT("\t\tCell: %i/%i (%i)\n"), CellNodes, InputMinimumCells, TotalCellNodes);
		Builder.Appendf(TEXT("\t\tNull: %i (%i)\n"), NullNodes, TotalNullNodes);
		Builder.Append(TEXT("\tDiscarded Nodes:\n"));
		Builder.Appendf(TEXT("\t\tOut Of Bounds: %i (%i)\n"), DiscardedOutOfBoundsNodes, TotalDiscardedOutOfBoundsNodes);
		Builder.Appendf(TEXT("\t\tIntersecting: %i (%i)\n"), DiscardedIntersectingNodes, TotalDiscardedIntersectingNodes);

		UE_LOG(LogNexusProcGen, Log, TEXT("%s"), Builder.ToString());
	}

private:
	/** Input minimum cell count captured from the organ. */
	int InputMinimumCells = 0;

	/** Input maximum cell count captured from the organ. */
	int InputMaximumCells = 0;

	/** Input maximum retry count captured from the organ. */
	int InputMaximumRetries = 0;

	/** Retries consumed so far. */
	int RetryCount = 0;

	/** Cells placed on the current attempt; reset by Retry. */
	int CellNodes = 0;

	/** Null-terminators placed on the current attempt; reset by Retry. */
	int NullNodes = 0;

	/** Out-of-bounds discards on the current attempt. */
	int DiscardedOutOfBoundsNodes = 0;

	/** Intersection-rejected discards on the current attempt. */
	int DiscardedIntersectingNodes = 0;

	/** Cells placed across every attempt. */
	int TotalCellNodes = 0;

	/** Null-terminators placed across every attempt. */
	int TotalNullNodes = 0;

	/** Out-of-bounds discards across every attempt. */
	int TotalDiscardedOutOfBoundsNodes = 0;

	/** Intersection-rejected discards across every attempt. */
	int TotalDiscardedIntersectingNodes = 0;

	/** Organ display name used in log output. */
	FString DisplayName;
};