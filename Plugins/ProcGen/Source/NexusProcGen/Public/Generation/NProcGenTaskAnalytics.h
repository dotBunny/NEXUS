// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Types/NIterationCounter.h"

struct FNOrganGraphBuilderAnalytics
{
	FString Name;
	
	double StartTime;
	double EndTime;
	double Duration;
	
	int Iterations = 1;
	
	FNIterationCounter AddNullNodes;
	FNIterationCounter AddCellNodes;
	
	FNIterationCounter DiscardOutOfBoundsStart;
	FNIterationCounter DiscardWorldCollidingStart;
	
	FNIterationCounter DiscardOutOfBoundsCellNode;
	FNIterationCounter DiscardIntersectingCellNode;
	FNIterationCounter DiscardWorldCollidingCellNode;
	
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
	}
};

class FNProcGenTaskAnalytics
{
public:
	void TaskGraphCreationStart();
	void TaskGraphCreationFinish();
	
	void CreateWorldContextStart();
	void CreateWorldContextFinish();
	
	void ProcessWorldContextStart();
	void ProcessWorldContextFinish();
	
	int OrganGraphBuilderCreate();
	void OrganGraphBuilderStart(int32 Index);
	void OrganGraphBuilderFinish(int32 Index);
	
	void OrganGraphBuilder_Init(int32 Index, const FString& Name, int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount);
	
	void OrganGraphBuilder_AddNullNode(int32 Index);
	void OrganGraphBuilder_AddCellNode(int32 Index);
	
	void OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index);
	void OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index);
	
	void OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index);
	void OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index);
	void OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index);
	
	void OrganGraphBuilder_NextIteration(int32 Index);
	
private:

#if !UE_BUILD_SHIPPING	
	double TaskGraphCreationStartTime = 0;
	double TaskGraphCreationEndTime = 0;
	double TaskGraphCreationDuration = 0;
	
	double CreateWorldContextStartTime = 0;
	double CreateWorldContextEndTime = 0;
	double CreateWorldContextDuration = 0;
	
	double ProcessWorldContextStartTime = 0;
	double ProcessWorldContextEndTime = 0;
	double ProcessWorldContextDuration = 0;
	
	TArray<FNOrganGraphBuilderAnalytics> OrganGraphBuilderAnalytics;
#endif // !UE_BUILD_SHIPPING
};