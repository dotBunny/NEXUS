// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenTaskTimer.h"
#include "Types/NIterationCounter.h"

struct FNOrganGraphBuilderAnalytics
{
	FString Name;
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
	
	int Iterations = 1;
	
	FNIterationCounter AddNullNodes;
	FNIterationCounter AddCellNodes;
	
	FNIterationCounter DiscardOutOfBoundsStart;
	FNIterationCounter DiscardWorldCollidingStart;
	
	FNIterationCounter DiscardOutOfBoundsCellNode;
	FNIterationCounter DiscardIntersectingCellNode;
	FNIterationCounter DiscardWorldCollidingCellNode;
	FNIterationCounter DiscardExistingNodeWorldCollidingCellNode;
	
	void NextIteration()
	{
		Iterations++;
		AddNullNodes.NextIteration();
		AddCellNodes.NextIteration();
		
		DiscardOutOfBoundsStart.NextIteration();
		DiscardWorldCollidingStart.NextIteration();
		
		DiscardOutOfBoundsCellNode.NextIteration();
		DiscardIntersectingCellNode.NextIteration();
		DiscardWorldCollidingCellNode.NextIteration();
		DiscardExistingNodeWorldCollidingCellNode.NextIteration();
	}
};
