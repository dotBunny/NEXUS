// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenMinimal.h"

struct FNProcGenGraphBuilderAnalytics
{

	
	
	void AddedNullNode()
	{
		NullNodes++;
		TotalNullNodes++;
	}
	void AddedCellNode()
	{
		CellNodes++;
		TotalCellNodes++;
	}
	void DiscardOutOfBounds()
	{
		DiscardedOutOfBoundsNodes++;
	}
	void DiscardIntersecting()
	{
		DiscardedIntersectingNodes++;
	}
	void Retry()
	{
		RetryCount++;
		CellNodes = 0;
		NullNodes = 0;
	}
	
	void Init(FString Name, const int MinimumCells, const int MaximumCells, const int MaximumRetries)
	{
		DisplayName = Name;
		InputMinimumCells = MinimumCells;
		InputMaximumCells = MaximumCells;
		InputMaximumRetries = MaximumRetries;
	}
	
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
	int InputMinimumCells = 0;
	int InputMaximumCells = 0;
	int InputMaximumRetries = 0;
	
	int RetryCount = 0;
	
	int CellNodes = 0;
	int NullNodes = 0;
	int DiscardedOutOfBoundsNodes = 0;
	int DiscardedIntersectingNodes = 0;
	
	int TotalCellNodes = 0;
	int TotalNullNodes = 0;
	int TotalDiscardedOutOfBoundsNodes = 0;
	int TotalDiscardedIntersectingNodes = 0;
	
	FString DisplayName;
	
};